#ifndef AVAILABLE_METRICS_H
#define AVAILABLE_METRICS_H

#include "papi.h"

#define MAX_LENGTH_ALIAS 100

static int *Events = NULL;

static void compute_metric(char** metric_names, aggregators_t *values, int size, int update)
{
  int retval;
  long long value;

  if(update) {
    Events = malloc(size * sizeof(int));
    for(int i = 0; i < size; ++i) {
      // if(Events) {
      //   free(Events);
      // }
      retval = PAPI_event_name_to_code(metric_names[i], &Events[i]);
      if (retval != PAPI_OK) {
        fprintf(stderr, "Invalid event name: %s\n", metric_names[i]);
      }
    }
  }

  for(int i = 0; i < size; ++i) {
    if (PAPI_start_counters(&Events[i], 1) != PAPI_OK) {
      fprintf(stderr, "Error in PAPI_start_counters\n");
    }

    if (PAPI_stop_counters(&value, 1) != PAPI_OK) {
      fprintf(stderr, "Error in PAPI_stop_counters\n");
    }

    values[i].min = (double) value;
    values[i].max = (double) value;
    values[i].sum = (double) value;
  }


  // int nprocs, rank;
  // MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  // MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  // for(int i = 0; i < size; ++i) {
  //   values[i].min = (double) rank;
  //   values[i].max = (double) rank;
  //   values[i].sum = (double) rank;
  // }
}

#endif
