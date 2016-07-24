#ifndef METRICS_CRAWLER_H
#define METRICS_CRAWLER_H

typedef struct _aggregators_t aggregators_t;

void initialize_metrics_crawler();

void initialize_metrics_crawler_number_from_file(metrics_aggregator_t *aggregator, long *nr, char* filename);

void initialize_metrics_crawler_number_from_memory(metrics_aggregator_t *aggregator, long *nr);

void metrics_crawler_results_memory(metrics_aggregator_t *aggregator, aggregators_t *result);

int metrics_crawler_results_file(metrics_aggregator_t *aggregator, aggregators_t *result, char *filename);

#endif