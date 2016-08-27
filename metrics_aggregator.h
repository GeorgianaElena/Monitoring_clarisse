#ifndef METRICS_AGGREGATOR_H
#define METRICS_AGGREGATOR_H

#include "mpi.h"
#include "stdio.h"
#include "sys/stat.h"
#include "sys/types.h"
#include "inttypes.h"
#include "unistd.h"
#include "evpath.h"
#include "stdbool.h"
#include "uthash.h"
#include "pthread.h"

#include "maggs_types.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

#define MAX_FILENAME_LENGTH 100
#define ADDRESS_SIZE 2048

////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef BENCHMARKING
typedef struct
{
  UT_hash_handle hh;
  uint64_t count;
  uint64_t ts;
} h_timestamp_t;
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _node_state_t
{
  char parent_addr[ADDRESS_SIZE];
  char own_addr[ADDRESS_SIZE];

  EVstone multi_stone;
  EVstone bridge_stone;
  EVstone split_stone;
  EVstone terminal_stone;

  /* Valid only for root */
  EVstone agreg_terminal_stone;

  CManager conn_mgr;
} node_state_t;

////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _aggregators_t
{
  double min;
  double max;
  double sum;
} aggregators_t, *aggregators_t_ptr;

////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _metrics_t
{
  long metrics_nr;
  aggregators_t *gather_info;
  int max_degree;
  int update_file;
  int parent_rank;
  int nprocs;
  long timestamp;
  int quit;
#ifdef BENCHMARKING
  double start_time;
#endif
} metrics_t, *metrics_t_ptr;

////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _metrics_aggregator_t
{
  char aliases_file[MAX_FILENAME_LENGTH];

#ifdef BENCHMARKING
  uint64_t max_timestamps;
#endif

  int max_degree;

  useconds_t pulse_interval;

  FILE *metrics_file;

  char **desired_metrics;

  long total_metrics;

  long old_nr_of_metrics;

  node_state_t current_state;

#ifdef BENCHMARKING
  double *benchmarking_results;
#else
  sys_metric_t *monitoring_results;
#endif

  pthread_mutex_t glock;
  pthread_cond_t cond_root_finished;
  uint8_t root_finished;
  
  pthread_cond_t results_cond;
  pthread_mutex_t results_lock;
  uint8_t results_available;

#ifdef BENCHMARKING
  FILE *results;
#else
  uint8_t leafs_finished;
  h_timestamp_t *hash_ts;
#endif
  MPI_Comm comm_leafs;
} metrics_aggregator_t;

////////////////////////////////////////////////////////////////////////////////////////////////////

void initialize_metrics_aggregator(metrics_aggregator_t *aggregator, char *a_f, uint64_t ts_nr,
                                   int d, unsigned int pulse_i);
void destroy_metrics_aggregator(metrics_aggregator_t *aggregator);

int final_result(CManager cm, void *vevent, void *client_data, attr_list attrs);
int compute_own_metrics(CManager cm, void *vevent, void *client_data, attr_list attrs);
void set_stones_actions(metrics_aggregator_t *aggregator);
void *start_communication(void *aggregator);
void compute_evpath_addr(metrics_aggregator_t *aggregator, char *addr);
void start_leaf_thread(metrics_aggregator_t *aggregator);

////////////////////////////////////////////////////////////////////////////////////////////////////
#endif
