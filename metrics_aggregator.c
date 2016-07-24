#include "metrics_aggregator.h"
#include "helpers.h"
#include "metrics_crawler.h"
#include "cod.h"
#include "storage.h"

#include "papi.h"

#define ROOT 0

////////////////////////////////////////////////////////////////////////////////////////////////////

/*EvPath vectors*/
static FMField aggregators_field_list[] = 
{
  {"min", "float", sizeof(long), FMOffset(aggregators_t_ptr, min)},
  {"max", "float", sizeof(long), FMOffset(aggregators_t_ptr, max)},
  {"sum", "float", sizeof(long), FMOffset(aggregators_t_ptr, sum)},
  {NULL, NULL}
};

static FMField metrics_field_list[] =
{
  {"metrics_nr", "integer", sizeof(long), FMOffset(metrics_t_ptr, metrics_nr)},
  {"gather_info", "aggregators_t[metrics_nr]", sizeof(aggregators_t), 
    FMOffset(metrics_t_ptr, gather_info)},
  {"max_degree", "integer", sizeof(int), FMOffset(metrics_t_ptr, max_degree)},
  {"update_file", "integer", sizeof(int), FMOffset(metrics_t_ptr, update_file)},
  {"parent_rank", "integer", sizeof(int), FMOffset(metrics_t_ptr, parent_rank)},
  {"nprocs", "integer", sizeof(int), FMOffset(metrics_t_ptr, nprocs)},
  {"timestamp", "integer", sizeof(long), FMOffset(metrics_t_ptr, timestamp)},
  {"quit", "integer", sizeof(int), FMOffset(metrics_t_ptr, quit)},
#ifdef BENCHMARKING
  {"start_time", "float", sizeof(long), FMOffset(metrics_t_ptr, start_time)},
#endif
  {NULL, NULL}
};

static FMStructDescRec metrics_format_list[] =
{
  {"metrics_t", metrics_field_list, sizeof(metrics_t), NULL},
  {"aggregators_t", aggregators_field_list, sizeof(aggregators_t), NULL},
  {NULL, NULL}
};

static FMStructDescList queue_list[] = {metrics_format_list, NULL};

////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////

void initialize_metrics_aggregator(metrics_aggregator_t *aggregator, char *a_f,
                                   uint64_t ts_nr, int d, unsigned int pulse_i)
{
  strcpy(aggregator->aliases_file, a_f);
#ifdef BENCHMARKING
  aggregator->max_timestamps = ts_nr;
 #endif
  aggregator->max_degree = d;
  aggregator->pulse_interval = pulse_i;

  pthread_mutex_init(&aggregator->glock, NULL);
  pthread_mutex_init(&aggregator->results_lock, NULL);
  pthread_cond_init(&aggregator->cond_root_finished, NULL);
  aggregator->root_finished = 0;

#ifdef BENCHMARKING
  aggregator->results = NULL;
#else
  aggregator->leafs_finished = 0;
  aggregator->hash_ts = NULL;
#endif

  aggregator->metrics_file = NULL;
  aggregator->total_metrics = 0;
  aggregator->old_nr_of_metrics = 0;
  aggregator->desired_metrics = NULL;

#ifdef BENCHMARKING
  aggregator->benchmarking_results = NULL;
#else
  aggregator->monitoring_results = NULL;
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void destroy_metrics_aggregator(metrics_aggregator_t *aggregator)
{
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  /* Destroy desired_metrics */
  if(aggregator->desired_metrics) {
    for(int i = 0; i < aggregator->old_nr_of_metrics; ++i) {
        free(aggregator->desired_metrics[i]);
        aggregator->desired_metrics[i] = NULL;
    }
    free(aggregator->desired_metrics);
    aggregator->desired_metrics = NULL;
  }

  /* Shutdown PAPI */
  PAPI_shutdown();

  /* Destroy stones */
  if(!is_leaf(aggregator)) {
    EVdestroy_stone(aggregator->current_state.conn_mgr,
                    aggregator->current_state.multi_stone);
    EVdestroy_stone(aggregator->current_state.conn_mgr,
                    aggregator->current_state.split_stone);
    EVdestroy_stone(aggregator->current_state.conn_mgr,
                    aggregator->current_state.terminal_stone);
    if(rank == ROOT) {
      EVdestroy_stone(aggregator->current_state.conn_mgr,
                      aggregator->current_state.agreg_terminal_stone);
    }
  }

  EVdestroy_stone(aggregator->current_state.conn_mgr,
                  aggregator->current_state.bridge_stone);

  /* Destroy benchmarking / monitoring results */
#ifdef BENCHMARKING
  free(aggregator->benchmarking_results);
#else
  free(aggregator->monitoring_results);
#endif

  /* Destroy sync objects*/
  pthread_mutex_destroy(&aggregator->glock);
  pthread_mutex_destroy(&aggregator->results_lock);
  pthread_cond_destroy(&aggregator->cond_root_finished);

  /* Destroy hashts */
#ifndef BENCHMARKING
  h_timestamp_t *tmp;
  h_timestamp_t *curr_ts;
  HASH_ITER(hh, aggregator->hash_ts, curr_ts, tmp) {
    HASH_DEL(aggregator->hash_ts, curr_ts);
    free(curr_ts);
  }
#endif
  free_storage();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void start_leaf_thread(metrics_aggregator_t *aggregator)
{
  pthread_t tid;
  pthread_attr_t tattr;
  int ret;

  ret = pthread_attr_init(&tattr);
  if (ret != 0) {
    perror("Attribute init failed for leafs");
    exit(EXIT_FAILURE);
  }

  ret = pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED);
  if (ret != 0) {
    perror("Setting detached state for leafs thread failed");
    exit(EXIT_FAILURE);
  }

  ret = pthread_create(&tid, &tattr, start_communication, (void *)aggregator);
  if (ret != 0) {
    perror("Creation of leafs thread failed");
    exit(EXIT_FAILURE);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

/* Allocate stones and compute split stone address for non leafs nodes */
void compute_evpath_addr(metrics_aggregator_t *aggregator, char *addr)
{
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  char *string_list_addr;

  aggregator->current_state.conn_mgr = CManager_create();
  CMlisten(aggregator->current_state.conn_mgr);

  aggregator->current_state.multi_stone = EValloc_stone(aggregator->current_state.conn_mgr);

  aggregator->current_state.split_stone = EValloc_stone(aggregator->current_state.conn_mgr);

  aggregator->current_state.terminal_stone = EValloc_stone(aggregator->current_state.conn_mgr); 

  aggregator->current_state.bridge_stone = EValloc_stone(aggregator->current_state.conn_mgr);

  if(rank == ROOT) {
    aggregator->current_state.agreg_terminal_stone = EValloc_stone(aggregator->current_state.conn_mgr);
  }

  string_list_addr = attr_list_to_string(CMget_contact_list(aggregator->current_state.conn_mgr));
  sprintf(addr, "%d:%s", aggregator->current_state.split_stone, string_list_addr);
  memcpy(aggregator->current_state.own_addr, addr, ADDRESS_SIZE);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

/* Set stones actions */
void set_stones_actions(metrics_aggregator_t *aggregator)
{
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if(rank != ROOT) {
    /* Create and start leafs CManager*/
    if(is_leaf(aggregator)) {
      aggregator->current_state.conn_mgr = CManager_create();
      CMlisten(aggregator->current_state.conn_mgr);
      aggregator->current_state.bridge_stone = EValloc_stone(aggregator->current_state.conn_mgr);
    }

    EVstone remote_stone;
    CManager cm = aggregator->current_state.conn_mgr;
    EVstone output = aggregator->current_state.bridge_stone;
    char string_list[ADDRESS_SIZE];

    if (sscanf(aggregator->current_state.parent_addr, "%d:%s",
               &remote_stone, &string_list[0]) != 2) {
      printf("Bad argument \"%s\"\n", aggregator->current_state.parent_addr);
      exit(0);
    }

    attr_list contact_list = attr_list_from_string(string_list); 

    EVassoc_bridge_action(cm, output, contact_list, remote_stone);

    if(!is_leaf(aggregator)) {
      EVstone_set_output(cm, aggregator->current_state.multi_stone, 0, output);
    }

  } else {

    EVassoc_terminal_action(aggregator->current_state.conn_mgr, aggregator->current_state.agreg_terminal_stone,
                            metrics_format_list, final_result, (void *) aggregator);

    EVstone_set_output(aggregator->current_state.conn_mgr, aggregator->current_state.multi_stone, 0,
                       aggregator->current_state.agreg_terminal_stone);
  }

  /* Create multistone */
  if(!is_leaf(aggregator)) {
    CManager cm = aggregator->current_state.conn_mgr;

    EVstone terminal_stone;

    terminal_stone = aggregator->current_state.terminal_stone;

    EVassoc_terminal_action(aggregator->current_state.conn_mgr, terminal_stone,
                            metrics_format_list, compute_own_metrics, (void *) aggregator);

    EVaction split_action = EVassoc_split_action(cm, aggregator->current_state.split_stone, NULL);

    EVaction_add_split_target(cm, aggregator->current_state.split_stone, split_action, terminal_stone);

    EVaction_add_split_target(cm, aggregator->current_state.split_stone, split_action,
                              aggregator->current_state.multi_stone);

    char *mq = create_multityped_action_spec(queue_list, multi_func);

    EVassoc_multi_action(aggregator->current_state.conn_mgr, aggregator->current_state.multi_stone, mq, NULL);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

/* Final aggregated system state */
int final_result(CManager cm, void *vevent, void *client_data, attr_list attrs)
{
  int nprocs, rank;
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  metrics_t_ptr event = vevent;
  int procs_done = 0;
  int pulse = 0;

  metrics_aggregator_t *aggregator = (metrics_aggregator_t *) client_data;

#ifdef BENCHMARKING
  double end_time = MPI_Wtime();
  procs_done = (event->timestamp == (aggregator->max_timestamps - 1));

  if(pulse == 0) {
    aggregator->benchmarking_results = (long *) calloc (aggregator->max_timestamps, sizeof(long));
  }

  aggregator->benchmarking_results[pulse] = end_time - event->start_time;
#else
  if(pulse == 0) {
    pthread_mutex_lock(&aggregator->results_lock);
    aggregator->monitoring_results = (sys_metric_t *) calloc (event->metrics_nr, sizeof(sys_metric_t));
    pthread_mutex_unlock(&aggregator->results_lock);
  } else if (event->update_file) {
    pthread_mutex_lock(&aggregator->results_lock);
    free(aggregator->monitoring_results);
    aggregator->monitoring_results = (sys_metric_t *) calloc(event->metrics_nr, sizeof(sys_metric_t));
    pthread_mutex_unlock(&aggregator->results_lock);
  }

  pthread_mutex_lock(&aggregator->results_lock);
  for(int i = 0; i < event->metrics_nr; ++i) {
    aggregator->monitoring_results[i].min = event->gather_info[i].min;
    aggregator->monitoring_results[i].max = event->gather_info[i].max;
    aggregator->monitoring_results[i].avg = (double) event->gather_info[i].sum / nprocs;
  }
  pthread_mutex_unlock(&aggregator->results_lock);
#endif

  ++pulse;

  if(procs_done || event->quit || !event->metrics_nr) {
#ifdef BENCHMARKING
    printf("Writing in file benchmarking results\n");

    for (long i = 0; i < aggregator->max_timestamps; ++i) {
      fprintf(aggregator->results, "%ld %ld\n", i, aggregator->benchmarking_results[i]);
    }

    fclose(aggregator->results);
    free(aggregator->benchmarking_results);
#endif

    pthread_mutex_lock(&aggregator->glock);
    aggregator->root_finished = 1;
    pthread_cond_signal(&aggregator->cond_root_finished);
    pthread_mutex_unlock(&aggregator->glock);
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

/* Compute metrics values */
void get_metrics_values(metrics_aggregator_t *aggregator, void *vevent, int counter, metrics_t *data)
{
  int rank, nprocs;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

  metrics_t_ptr event = vevent;

  data->max_degree = aggregator->max_degree;

  data->update_file = event->update_file;

  data->timestamp = counter;

  data->parent_rank = get_parent(aggregator);

  data->nprocs = nprocs;

  data->quit = event->quit;

#ifdef BENCHMARKING
  data->start_time = -1;
#endif

  if(event->update_file) {
    initialize_metrics_crawler_number_from_file(aggregator, &data->metrics_nr, aggregator->aliases_file);
    data->gather_info = malloc(data->metrics_nr * sizeof(aggregators_t));
    metrics_crawler_results_file(aggregator, data->gather_info, aggregator->aliases_file);
  } else {
    initialize_metrics_crawler_number_from_memory(aggregator, &data->metrics_nr);
    metrics_crawler_results_memory(aggregator, data->gather_info);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

/* Compute metrics for current node and submit them to the multistone located on same node */
int compute_own_metrics(CManager cm, void *vevent, void *client_data, attr_list attrs)
{
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  metrics_t_ptr event = vevent;
  metrics_aggregator_t *aggregator = (metrics_aggregator_t *) client_data;

  static int counter = 0;
  static int initialized = 0;
  static int first_pulse = 1;
  static metrics_t data;
  static EVsource source;
  uint64_t count_ts = 0;
#ifdef BENCHMARKING
  static int *count_timestamps;
#endif

  if(!initialized) {
#ifdef BENCHMARKING
    count_timestamps = calloc(aggregator->max_timestamps, sizeof(int));
#endif
    source = EVcreate_submit_handle(aggregator->current_state.conn_mgr, aggregator->current_state.multi_stone,
                                    metrics_format_list);
    initialized = 1;
  }

#ifdef BENCHMARKING
  count_timestamps[event->timestamp]++;
  count_ts = count_timestamps[counter];
#else
  h_timestamp_t *found;
  HASH_FIND(hh, aggregator->hash_ts, &event->timestamp, sizeof(uint64_t), found);
  if (!found) {
    found = malloc(sizeof(h_timestamp_t));
    found->ts = event->timestamp;
    found->count = 1;
    HASH_ADD(hh, aggregator->hash_ts, ts, sizeof(uint64_t), found);
  } else {
    found->count++;
  }

  HASH_FIND(hh, aggregator->hash_ts, &counter, sizeof(uint64_t), found);
  count_ts = found->count;
#endif

  if(count_ts == get_degree_node(aggregator)) {
    /* If it's the first pulsation or the metrics requested 
       have changed, compute own metric in current pulsation */
    if(first_pulse || event->update_file || event->quit) {
      get_metrics_values(aggregator, vevent, counter, &data);
      first_pulse = 0;
    }

    EVsubmit(source, &data, NULL);
    ++counter;

#ifdef BENCHMARKING
    if(data.timestamp == aggregator->max_timestamps - 1) {
      return 0;
    }
#else
    HASH_DEL(aggregator->hash_ts, found);
    free(found);
    if (event->quit) {
      return 0;
    }
#endif

    /* Start computing metrics for the next pulsation */
    get_metrics_values(aggregator, vevent, counter, &data);
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

/* Start propagating metrics values up through the tree topology */
void *start_communication(void *aggregator)
{
  metrics_aggregator_t *agg = (metrics_aggregator_t *) aggregator;

  int rank, nprocs;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

  EVsource source = EVcreate_submit_handle(agg->current_state.conn_mgr, agg->current_state.bridge_stone,
                                           metrics_format_list);

  int counter = 0;

  metrics_t data;

  data.max_degree = agg->max_degree;

  data.parent_rank = get_parent(aggregator);

  data.nprocs = nprocs;

  data.quit = 0;

  /* Send data periodically with different timestamps */
  while (!data.quit) {
#ifdef BENCHMARKING
    if (counter == agg->max_timestamps - 1) {
      data.quit = 1;
    }
#else
    pthread_mutex_lock(&agg->glock);
    if (agg->leafs_finished) {
      data.quit = 1;
    } else {
      data.quit = 0;
    }
    pthread_mutex_unlock(&agg->glock);
#endif

    data.timestamp = counter;

    data.parent_rank = get_parent(agg);

    data.nprocs = nprocs;

    initialize_metrics_crawler_number_from_file(agg, &data.metrics_nr, agg->aliases_file);

    data.gather_info = malloc(data.metrics_nr * sizeof(aggregators_t));

    data.update_file = metrics_crawler_results_file(agg, data.gather_info, agg->aliases_file);

#ifdef BENCHMARKING
    double start_time;
    start_time = MPI_Wtime();
    data.start_time = start_time;
#endif

    EVsubmit(source, &data, NULL);

    free(data.gather_info);

    ++counter;

    usleep(agg->pulse_interval);
  }

  return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////