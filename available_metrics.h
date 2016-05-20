#ifndef AVAILABLE_METRICS_H
#define AVAILABLE_METRICS_H

#include "papi.h"

#define MAX_LENGTH_ALIAS 100

static void compute_metric(char** metric_names, aggregators_t *values, int *Events, int size)
{
  long long value;
  int retval;

  for(int i = 0; i < size; ++i) {
    retval = PAPI_event_name_to_code(metric_names[i], &Events[i]);
    if (retval != PAPI_OK) {
      fprintf(stderr, "Invalid event name: %s\n", metric_names[i]);
    }


    if (PAPI_start_counters(&Events[i], 1) != PAPI_OK) {
      fprintf(stderr, "Error in PAPI_start_counters, event: %s\n", metric_names[i]);
    }

    if (PAPI_stop_counters(&value, 1) != PAPI_OK) {
      fprintf(stderr, "Error in PAPI_stop_counters, event: %s\n", metric_names[i]);
    }

    values->min = value;
    values->max = value;
    values->sum = value;
  }

  // PAPI_shutdown();
}

#endif
