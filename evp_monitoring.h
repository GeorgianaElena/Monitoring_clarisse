#ifndef EVP_MONITORING_H
#define EVP_MONITORING_H

 #include "maggs_types.h"

#include "inttypes.h"
#include "unistd.h"

typedef struct _metrics_aggregator_t metrics_aggregator_t;
typedef struct _sys_metric_t sys_metric_t;

////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _evp_monitoring_t
{
  metrics_aggregator_t *aggregator;
} evp_monitoring_t;

////////////////////////////////////////////////////////////////////////////////////////////////////

void monitoring_initialize(evp_monitoring_t *, char *,  uint64_t, int, unsigned int);

void monitoring_finalize(evp_monitoring_t *);

#ifndef BENCHMARKING

/* Returns the latest available metrics values. This function may block the calling thread
 * until the first pulsation results are available.
 */
void return_results(evp_monitoring_t *, sys_metric_t *);

/* This function *must* be called every time the input metrics file is dinamically changed
 * so that the subsequent 'return_results' call to return the updated metrics results.
 */
void monitoring_file_updated(evp_monitoring_t *mon);
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
#endif