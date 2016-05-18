#ifndef METRICS_AGGREGATOR_H
#define METRICS_AGGREGATOR_H

#include "mpi.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "sys/stat.h"
#include "sys/types.h"
#include "fcntl.h"
#include "unistd.h"
#include "stdbool.h"
#include "stdint.h"

#define ADDRESS_SIZE 2048
#define MAX_FILENAME_LENGTH 100
#define BENCHMARKING

#ifdef BENCHMARKING
FILE *results = NULL;
#endif

#ifndef BENCHMARKING
FILE *aggregated_metrics = NULL;
#endif


char aliases_file[MAX_FILENAME_LENGTH];
int DEGREE;
uint64_t MAX_TIMESTAMPS;
useconds_t pulse_interval;

typedef struct _aggregators_t{
  double min;
  double max;
  double sum;
} aggregators_t, *aggregators_t_ptr;

typedef struct _metrics_t{
  long metrics_nr;
  aggregators_t *gather_info;
  int update_file;
  long timestamp;
#ifdef BENCHMARKING
  double start_time;
#endif
} metrics_t;

static int is_leaf();
static int get_parent();
static int get_degree_node();

void recv_event_from_childern(metrics_t *event);
void send_event_to_parent(metrics_t *event);
void start_communication();
void compute_and_aggregate();

#endif
