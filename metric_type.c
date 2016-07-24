#include "metric_type.h"
#include "metrics_aggregator.h"
#include "storage.h"

#include "stdio.h"
#include "papi.h"

int *Events = NULL;

void realloc_papi_events(int size)
{
  // if(Events != NULL) {
  //   free(Events);
  // }
  Events = malloc(size * sizeof(int));
}

void compute_metric(char** metric_names, aggregators_t *values, int size, int update)
{
  int retval;
  long long value = 0;
  int papi_metrics_nr = 0;

  if(update) {
    realloc_papi_events(size);
  }

  for(int i = 0; i < size; ++i) {
    long (*func)();
    func = get_value(metric_names[i]);
    if(func) {
      /* The metric passed is a custom one */
      value = func();
    } else {
      /* The metric passed is PAPI*/
      retval = PAPI_event_name_to_code(metric_names[i], &Events[papi_metrics_nr]);
      if (retval != PAPI_OK) {
        fprintf(stderr, "Metric: %s, not supported by PAPI lib", metric_names[i]);
        return;
      }
      if (PAPI_start_counters(&Events[papi_metrics_nr], 1) != PAPI_OK) {
        fprintf(stderr, "Error in PAPI_start_counters\n");
        return;
      }

      ++papi_metrics_nr;
      if (PAPI_stop_counters(&value, 1) != PAPI_OK) {
        fprintf(stderr, "Error in PAPI_stop_counters\n");
        return;
      }
    }

    values[i].min = (long) value;
    values[i].max = (long) value;
    values[i].sum = (long) value;
  }
}