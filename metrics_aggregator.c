#include "metrics_aggregator.h"
#include "metrics_crawler.h"
#include "helpers.h"
#include "inttypes.h"

static node_state_t current_state;

#ifdef BENCHMARKING
FILE *results;
#endif

int main(int argc, char **argv)
{
  int rank, nprocs, provided;

  MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

  if (argc < 4) {

    if(rank == 0) {
      fprintf(stderr, "----------------------------\n"
                      "Invalid number of arguments\n"
                      "----------------------------\n"
                      "Arguments: metrics aliases file\n"
                      "           maximum number of events\n"
                      "           maximum node degree\n");
    }

    MPI_Finalize();

    exit(0);
  }

  /* Metrics file */
  strcpy(aliases_file, argv[1]);
  /* Maximum number of events */
  sscanf(argv[2], "%" PRIu64, &MAX_TIMESTAMPS);
  /* Maximum node degree */
  sscanf(argv[3], "%d", &DEGREE);

/*----------------------------------------------------------------------------------*/

  /* Create leafs communicator for benchmarking*/
#ifdef BENCHMARKING
  create_leafs_comm();
#endif

  /* Start initialization */
  initialize_monitoring();

  /* Wait for all the processes to finish init phase */
  MPI_Barrier(MPI_COMM_WORLD);

  /* Fork a thread to handle the network input operations of a connection manager */
  if(!is_leaf()) {
    CMfork_comm_thread(current_state.conn_mgr);
  }

  /* Create EVPath stones and set actions */
  set_stones_actions();

  /* Wait for all the processes to setup stones */
  MPI_Barrier(MPI_COMM_WORLD);

  /* Keep alive main thread of not leaf processes */
  if(!is_leaf()) {
    pause();
  }

  /* Start aggregating metrics */
  start_communication();

  MPI_Finalize();

  return 0;
}

/* Final aggregated system state */
static int final_result(CManager cm, void *vevent, void *client_data, attr_list attrs)
{
  int nprocs, rank;
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  metrics_t_ptr event = vevent;

  /* Open for writing results file */
#ifdef BENCHMARKING
  char filename[MAX_FILENAME_LENGTH];
  sprintf(filename, "./results/results_%dnodes_%ldmetrics_%ddegree_%ld",
                     nprocs, event->metrics_nr, DEGREE, MAX_TIMESTAMPS);
  results = fopen(filename, "a+");

  double end_time, mintime;
  measure_time(&end_time, &mintime);

  int rank_comm_leafs;
  MPI_Comm_rank(comm_leafs, &rank_comm_leafs);

  if(rank_comm_leafs == 0) {
    fprintf(results, "min = %lf\n", end_time - mintime);
  }

  fclose(results);
#endif

  if(event->timestamp == MAX_TIMESTAMPS - 1) {
    printf("Process = %d stops\n", rank);
    MPI_Finalize();

    exit(0);
  }

  for(int i = 0; i < event->metrics_nr; ++i) {
    printf("-------------------------------------------"
           "-------------------------------------------\n"
           "%s    Min = %f    Max = %f    Average = %f\n",
            desired_metrics[i], event->gather_info[i].min,
            event->gather_info[i].max, event->gather_info[i].sum / nprocs);
  }

  return 0;
}

/* Compute metrics for current process */
static int compute_own_metrics(CManager cm, void *vevent, void *client_data, attr_list attrs)
{
  metrics_t_ptr event = vevent;

  int rank, nprocs;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

  static int *count_timestamps;

  static int initialized = 0;

  if(!initialized) {
    count_timestamps = calloc(MAX_TIMESTAMPS, sizeof(int));
    initialized = 1;
  }

  static int counter = 0;

  count_timestamps[event->timestamp]++;

  if(count_timestamps[counter] == get_degree_node()) {

    EVsource source = EVcreate_submit_handle(current_state.conn_mgr, current_state.multi_stone,
                                             metrics_format_list);

    initialize_metrics_crawler();

    metrics_t data;

    data.max_degree = DEGREE;

    data.update_file = event->update_file;

    data.timestamp = counter;

    data.parent_rank = get_parent();

    data.nprocs = nprocs;

    if(event->update_file) {
      initialize_metrics_crawler_number_from_file(&data.metrics_nr, aliases_file);
    } else {
      initialize_metrics_crawler_number_from_memory(&data.metrics_nr);
    }

    data.gather_info = malloc(data.metrics_nr * sizeof(aggregators_t));

    if(event->update_file) {
      metrics_crawler_results_file(data.gather_info, aliases_file);
    } else {
      metrics_crawler_results_memory(data.gather_info);
    }

    count_timestamps[counter] = 0;

    ++counter;

    EVsubmit(source, &data, NULL);

    if(data.timestamp == MAX_TIMESTAMPS - 1) {
      printf("Process = %d stops\n", rank);
      MPI_Finalize();

      exit(0);
    }

  }

  return 0;
}

/* Receive EVPath address of parent split stone through MPI */
void recv_addr_from_parent(char *addr)
{
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  MPI_Recv(addr, ADDRESS_SIZE, MPI_CHAR, get_parent(), 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

  printf("Process %d received from parent %s\n", rank, addr);
}

/* Send address to children through MPI */
void send_addr_to_children(char *addr)
{
  int rank, tag = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  for(int i = 1; i <= get_degree_node(); ++i) {
    MPI_Send(addr, ADDRESS_SIZE, MPI_CHAR, rank * DEGREE + i, tag, MPI_COMM_WORLD);
  }
}

/* Allocate stones and compute split stone address for not leafs procs */
void compute_evpath_addr(char *addr)
{
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  char *string_list_addr;

  current_state.conn_mgr = CManager_create();
  CMlisten(current_state.conn_mgr);

  current_state.multi_stone = EValloc_stone(current_state.conn_mgr);

  current_state.split_stone = EValloc_stone(current_state.conn_mgr);

  current_state.terminal_stone = EValloc_stone(current_state.conn_mgr); 

  current_state.bridge_stone = EValloc_stone(current_state.conn_mgr);

  if(rank == 0) {
    current_state.agreg_terminal_stone = EValloc_stone(current_state.conn_mgr);
  }

  string_list_addr = attr_list_to_string(CMget_contact_list(current_state.conn_mgr));

  sprintf(addr, "%d:%s", current_state.split_stone, string_list_addr);

  memcpy(current_state.own_addr, addr, ADDRESS_SIZE);
}

void initialize_monitoring()
{
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  /* Receive evpath address from parent node */
  if(rank != 0) {
    char addr[ADDRESS_SIZE];
    recv_addr_from_parent(addr);

    memcpy(current_state.parent_addr, addr, ADDRESS_SIZE);
  }

  /* Compute evpath address and send to children */
  if(!is_leaf()) {
    char myaddr[ADDRESS_SIZE];

    compute_evpath_addr(myaddr);

    send_addr_to_children(myaddr);
  }
}

/* Set stones actions */
void set_stones_actions()
{
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if(rank != 0) {

    if(is_leaf()) {
      current_state.conn_mgr = CManager_create();

      CMlisten(current_state.conn_mgr);

      current_state.bridge_stone = EValloc_stone(current_state.conn_mgr);
    }

    CManager cm = current_state.conn_mgr;

    EVstone output = current_state.bridge_stone;

    char string_list[ADDRESS_SIZE];

    EVstone remote_stone;

    if (sscanf(current_state.parent_addr, "%d:%s", &remote_stone, &string_list[0]) != 2) {
      printf("Bad argument \"%s\"\n", current_state.parent_addr);
      exit(0);
    }

    attr_list contact_list = attr_list_from_string(string_list); 

    EVassoc_bridge_action(cm, output, contact_list, remote_stone);

    EVstone_set_output(cm, current_state.multi_stone, 0, output);

  } else {

    EVassoc_terminal_action(current_state.conn_mgr, current_state.agreg_terminal_stone,
                            metrics_format_list, final_result, NULL);

    EVstone_set_output(current_state.conn_mgr, current_state.multi_stone, 0,
                       current_state.agreg_terminal_stone);
  }


  /* Create multistone */
  if(!is_leaf()) {
    CManager cm = current_state.conn_mgr;

    EVstone terminal_stone;

    terminal_stone = current_state.terminal_stone;

    EVassoc_terminal_action(current_state.conn_mgr, terminal_stone,
                          metrics_format_list, compute_own_metrics, NULL);

    EVaction split_action = EVassoc_split_action(cm, current_state.split_stone, NULL);

    EVaction_add_split_target(cm, current_state.split_stone, split_action, terminal_stone);

    EVaction_add_split_target(cm, current_state.split_stone, split_action,
                              current_state.multi_stone);

    char *mq = create_multityped_action_spec(queue_list, multi_func);

    EVassoc_multi_action(current_state.conn_mgr, current_state.multi_stone, mq, NULL);
  }
}

/* Start sending metrics value */
void start_communication()
{
  int rank, nprocs;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

  EVsource source = EVcreate_submit_handle(current_state.conn_mgr, current_state.bridge_stone,
                                           metrics_format_list);

  /* Populate the storage data stucture with the metrics known by the program so far */
  initialize_metrics_crawler();

  int counter = 0;

  metrics_t data;

  data.max_degree = DEGREE;

  data.parent_rank = get_parent();

  data.update_file = true;

  data.timestamp = counter;

  data.nprocs = nprocs;

  initialize_metrics_crawler_number_from_file(&data.metrics_nr, aliases_file);

  data.gather_info = malloc(data.metrics_nr * sizeof(aggregators_t));

  metrics_crawler_results_file(data.gather_info, aliases_file);

#ifdef BENCHMARKING
  double start_time, min_time;

  measure_time(&start_time, &min_time);
#endif

  EVsubmit(source, &data, NULL);


  /* Send data periodically with different timestamps */
  while (1) {

    ++counter;

    usleep(5000);

    data.timestamp = counter;

    data.parent_rank = get_parent();

    data.nprocs = nprocs;

    initialize_metrics_crawler_number_from_file(&data.metrics_nr, aliases_file);

    data.gather_info = malloc(data.metrics_nr * sizeof(aggregators_t));

    data.update_file = metrics_crawler_results_file(data.gather_info, aliases_file);

    EVsubmit(source, &data, NULL);

#ifdef BENCHMARKING
    measure_time(&start_time, &min_time);
#endif

    if(data.timestamp == MAX_TIMESTAMPS - 1) {
      printf("Process = %d stops\n", rank);
      MPI_Finalize();

      exit(0);
    }

  }
}