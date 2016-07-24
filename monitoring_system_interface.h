#ifndef EVP_MONITORING_H
#define EVP_MONITORING_H

#include "metrics_aggregator.h"
#include "inttypes.h"
#include "unistd.h"

typedef struct _evp_monitoring_t 
{
	metrics_aggregator_t aggregator;
} evp_monitoring_t;

void monitoring_initialize(evp_monitoring_t *, char *,  uint64_t, int, unsigned int);
void monitoring_finalize(evp_monitoring_t *);

#endif