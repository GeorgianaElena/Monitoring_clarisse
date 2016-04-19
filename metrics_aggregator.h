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
#include "evpath.h"
#include "stdbool.h"

#define DEGREE 2
#define ADDRESS_SIZE 2048
#define MAX_TIMESTAMPS 100
#define MAX_FILENAME_LENGTH 100

char aliases_file[MAX_FILENAME_LENGTH];

typedef struct _node_state_t{
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

typedef struct _aggregators_t{
  double min;
  double max;
  double sum;
} aggregators_t, *aggregators_t_ptr;

typedef struct _metrics_t{
  long metrics_nr;
  aggregators_t *gather_info;
  int max_degree;
  int update_file;
  int parent_rank;
  int nprocs;
  long timestamp;
} metrics_t, *metrics_t_ptr;

static FMField aggregators_field_list[] = 
{
  {"min", "float", sizeof(double), FMOffset(aggregators_t_ptr, min)},
  {"max", "float", sizeof(double), FMOffset(aggregators_t_ptr, max)},
  {"sum", "float", sizeof(double), FMOffset(aggregators_t_ptr, sum)},
  {NULL, NULL}
};

static FMField metrics_field_list[] =
{
  {"metrics_nr", "integer", sizeof(long), FMOffset(metrics_t_ptr, metrics_nr)},
  {"gather_info", "aggregators_t[metrics_nr]", sizeof(aggregators_t), FMOffset(metrics_t_ptr, gather_info)},
  {"max_degree", "integer", sizeof(int), FMOffset(metrics_t_ptr, max_degree)},
  {"update_file", "integer", sizeof(int), FMOffset(metrics_t_ptr, update_file)},
  {"parent_rank", "integer", sizeof(int), FMOffset(metrics_t_ptr, parent_rank)},
  {"nprocs", "integer", sizeof(int), FMOffset(metrics_t_ptr, nprocs)},
  {"timestamp", "integer", sizeof(long), FMOffset(metrics_t_ptr, timestamp)},
  {NULL, NULL}
};

static FMStructDescRec metrics_format_list[] =
{
  {"metrics_t", metrics_field_list, sizeof(metrics_t), NULL},
  {"aggregators_t", aggregators_field_list, sizeof(aggregators_t), NULL},
  {NULL, NULL}
};

static FMStructDescList queue_list[] = {metrics_format_list, NULL};

static int final_result(CManager cm, void *vevent, void *client_data, attr_list attrs);
static int compute_own_metrics(CManager cm, void *vevent, void *client_data, attr_list attrs);
static int is_leaf();
static int get_parent();
static int get_degree_node();

void recv_addr_from_parent(char *addr);
void send_addr_to_children(char *addr);
void initialize_monitoring();
void set_stones_actions();
void start_listening();
void start_communication();
void compute_evpath_addr(char *addr);

#endif