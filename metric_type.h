#ifndef METRIC_TYPE
#define METRIC_TYPE

typedef struct _aggregators_t aggregators_t;

void realloc_papi_events(int size);

void compute_metric(char** metric_names, aggregators_t *values, int size, int update);

#endif