#include "metrics_aggregator.h"
#include "metrics_crawler.h"
#include "helpers.h"
#include "inttypes.h"

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
                      "           maximum node degree\n"
                      "           pulsation interval (mircrosec)\n");
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
  sscanf(argv[4], "%u", &pulse_interval);

/*----------------------------------------------------------------------------------*/

#ifdef BENCHMARKING
  if(rank == 0) {
    char filename[MAX_FILENAME_LENGTH];
    char dirname[MAX_FILENAME_LENGTH];
    long nr_metrics;

    metrics_file = fopen (aliases_file, "r");
    if(!metrics_file) {
      perror("Error on opening metric aliases file");
      exit(-1);
    }

    if(fscanf(metrics_file, "%ld", &nr_metrics) == 0) {
      perror("Error on reading metrics file");
      exit(-1);
    }

    fclose(metrics_file);

    sprintf(dirname, "MPI_results");

    struct stat st = {0};
    if (stat(dirname, &st) == -1) {
      mkdir(dirname, S_IRWXU);
    }

    sprintf(filename, "./%s/mpi_%dnodes_%ddegree_%ldevents_%ldmetrics_%upulse",
                       dirname, nprocs, DEGREE, MAX_TIMESTAMPS, nr_metrics, pulse_interval);

    results = fopen(filename, "w");

    if(!results) {
      perror("Error oppening benchmarking results file");
      exit(-1);
    }
  }
#else
  if(rank == 0) {
    aggregated_metrics = fopen("aggregated_metrics", "w");

    if(!aggregated_metrics) {
      perror("Error oppening metrics results file");
      exit(-1);
    }
  }
#endif

  if(is_leaf()) {
    start_communication();
  } else {
    while(1) {
      compute_and_aggregate();
    }
  }

  MPI_Finalize();

  return 0;
}

/* Compute metrics for current process */
void compute_and_aggregate()
{
  int rank, nprocs;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

  static int counter = 0;

  initialize_metrics_crawler();

  metrics_t my_data;

  my_data.timestamp = counter;


  ++counter;

    metrics_t data;
    recv_event_from_childern(&data);

#ifdef BENCHMARKING
    my_data.start_time = data.start_time;
#endif

    if(data.update_file) {
      initialize_metrics_crawler_number_from_file(&my_data.metrics_nr, aliases_file);
    } else {
      initialize_metrics_crawler_number_from_memory(&my_data.metrics_nr);
    }

    my_data.gather_info = malloc(my_data.metrics_nr * sizeof(aggregators_t));

    if(data.update_file) {
      metrics_crawler_results_file(my_data.gather_info, aliases_file);
    } else {
      metrics_crawler_results_memory(my_data.gather_info);
    }

    my_data.update_file = data.update_file;

    for(int i = 0; i < data.metrics_nr; ++i) {
      if(my_data.gather_info[i].min > data.gather_info[i].min) {
        my_data.gather_info[i].min = data.gather_info[i].min;
      }

      if(my_data.gather_info[i].max < data.gather_info[i].max) {
        my_data.gather_info[i].max = data.gather_info[i].max;
      }

      my_data.gather_info[i].sum += data.gather_info[i].sum;
    }


  if(rank != 0) {
    send_event_to_parent(&my_data);

    if(my_data.timestamp == MAX_TIMESTAMPS - 1) {
      printf("Process = %d stops\n", rank);
      MPI_Finalize();

      exit(0);
    }
  } else {
#ifdef BENCHMARKING
    double end_time = MPI_Wtime();
    fprintf(results, "%ld %lf\n", my_data.timestamp, end_time - data.start_time);
#else
    for(int i = 0; i < my_data.metrics_nr; ++i) {
      fprintf(aggregated_metrics,
             "-------------------------------------------"
             "-------------------------------------------\n"
             "%s    Min = %f    Max = %f    Average = %f\n",
              desired_metrics[i], my_data.gather_info[i].min,
              my_data.gather_info[i].max, my_data.gather_info[i].sum / nprocs);
    }
#endif
    if(my_data.timestamp == MAX_TIMESTAMPS - 1) {
      printf("Process = %d stops\n", rank);
      fclose(results);
      MPI_Finalize();

      exit(0);
    }
  }
}

void recv_event_from_childern(metrics_t *event)
{
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  for(int i = 1; i <= get_degree_node(); ++i) {
    MPI_Recv(&event->metrics_nr, 1, MPI_LONG, rank * DEGREE + i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    if(i == 1) {
      event->gather_info = malloc(event->metrics_nr * sizeof(aggregators_t));

      for(int j = 0; j < event->metrics_nr; ++j) {
        MPI_Recv(&event->gather_info[j].min, 1, MPI_DOUBLE, rank * DEGREE + i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        MPI_Recv(&event->gather_info[j].max, 1, MPI_DOUBLE, rank * DEGREE + i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&event->gather_info[j].sum, 1, MPI_DOUBLE, rank * DEGREE + i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      }
    } else {
      for(int j = 0; j < event->metrics_nr; ++j) {
        double min, max, sum;

        MPI_Recv(&min, 1, MPI_DOUBLE, rank * DEGREE + i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&max, 1, MPI_DOUBLE, rank * DEGREE + i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&sum, 1, MPI_DOUBLE, rank * DEGREE + i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        if(event->gather_info[j].min > min) {
          event->gather_info[j].min = min;
        }

        if(event->gather_info[j].max < max) {
          event->gather_info[j].max = max;
        }

        event->gather_info[j].sum += sum;
      }
    }

    MPI_Recv(&event->update_file, 1, MPI_INT, rank * DEGREE + i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Recv(&event->timestamp, 1, MPI_LONG, rank * DEGREE + i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    if(i == 1) {
      MPI_Recv(&event->start_time, 1, MPI_DOUBLE, rank * DEGREE + i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    } else {
      double start_time;
      MPI_Recv(&start_time, 1, MPI_DOUBLE, rank * DEGREE + i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

      if(start_time < event->start_time) {
        event->start_time = start_time;
      }
    }
  }
}

void send_event_to_parent(metrics_t *event)
{
  int rank, tag = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  MPI_Send(&event->metrics_nr, 1, MPI_LONG, get_parent(), tag, MPI_COMM_WORLD);
  int i;
  for(i = 0; i < event->metrics_nr; ++i) {
    MPI_Send(&event->gather_info[i].min, 1, MPI_DOUBLE, get_parent(), tag, MPI_COMM_WORLD);
    MPI_Send(&event->gather_info[i].max, 1, MPI_DOUBLE, get_parent(), tag, MPI_COMM_WORLD);
    MPI_Send(&event->gather_info[i].sum, 1, MPI_DOUBLE, get_parent(), tag, MPI_COMM_WORLD);
  }

  MPI_Send(&event->update_file, 1, MPI_INT, get_parent(), tag, MPI_COMM_WORLD);
  MPI_Send(&event->timestamp, 1, MPI_LONG, get_parent(), tag, MPI_COMM_WORLD);
  MPI_Send(&event->start_time, 1, MPI_DOUBLE, get_parent(), tag, MPI_COMM_WORLD);
}



/* Start sending metrics value */
void start_communication()
{
  int rank, nprocs;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

  /* Populate the storage data stucture with the metrics known by the program so far */
  initialize_metrics_crawler();

  int counter = 0;

  metrics_t data;

  data.update_file = true;

  data.timestamp = counter;

  initialize_metrics_crawler_number_from_file(&data.metrics_nr, aliases_file);

  data.gather_info = malloc(data.metrics_nr * sizeof(aggregators_t));

  metrics_crawler_results_file(data.gather_info, aliases_file);


#ifdef BENCHMARKING
  double start_time;
  start_time = MPI_Wtime();

  data.start_time = start_time;
  // printf("%lf\n", data.start_time);
#endif

  send_event_to_parent(&data);


  /* Send data periodically with different timestamps */
  while (1) {

    ++counter;

    usleep(100000);

    data.timestamp = counter;

    initialize_metrics_crawler_number_from_file(&data.metrics_nr, aliases_file);

    data.gather_info = malloc(data.metrics_nr * sizeof(aggregators_t));

    data.update_file = metrics_crawler_results_file(data.gather_info, aliases_file);

#ifdef BENCHMARKING
  double start_time;
  start_time = MPI_Wtime();

  data.start_time = start_time;
    // printf("%lf\n", data.start_time);
#endif

    send_event_to_parent(&data);

    if(data.timestamp == MAX_TIMESTAMPS - 1) {
      printf("Process = %d stops\n", rank);
      MPI_Finalize();

      exit(0);
    }

  }
}