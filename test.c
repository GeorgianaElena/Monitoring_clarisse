#include "evp_monitoring.h"

#include "stdio.h"
#include "stdlib.h"
#include "mpi.h"
#include "string.h"

int main(int argc, char **argv)
{
  int rank, nprocs, provided;

  MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

/////////////////////////////////////////////////////////////////////////////////////////////////////

  if (argc < 5) {

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


  char a_f[50];
  uint64_t ts_nr;
  useconds_t pulse_i;
  int d;

  /* Metrics file */
  sscanf(argv[1], "%s", a_f);

  /* Maximum number of events */
  sscanf(argv[2], "%" PRIu64, &ts_nr);

  /* Maximum node degree */
  sscanf(argv[3], "%d", &d);

  /* Pulsation interval (microseconds) */
  sscanf(argv[4], "%u", &pulse_i);

///////////////////////////////////////////////////////////////////////////////////////////////////

  evp_monitoring_t mon;

  monitoring_initialize(&mon, a_f, ts_nr, d, pulse_i);

  /* Here could run the program to monitor */
  sleep(1);

#ifndef BENCHMARKING
  int num_metrics_file_txt = 1;
  sys_metric_t *res = (sys_metric_t *) calloc (num_metrics_file_txt, sizeof(sys_metric_t));

  if(rank == 0) {
    return_results(&mon, res);
    for(int i = 0; i < num_metrics_file_txt; ++i) {
      printf("min = %ld ", res[i].min);
      printf("max = %ld ", res[i].max);
      printf("avg = %lf\n", res[i].avg);
    }
  }
#endif

  sleep(2);

  monitoring_finalize(&mon);

  MPI_Finalize();

  return 0;
}
