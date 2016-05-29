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
#ifdef BENCHMARKING
  double start_time;
#endif
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
#ifdef BENCHMARKING
  {"start_time", "float", sizeof(double), FMOffset(metrics_t_ptr, start_time)},
#endif
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
static void initialize_monitoring();
static void set_stones_actions();
static void *start_communication();
static void compute_evpath_addr(char *addr);
static void start_leaf_thread();


#ifndef BENCHMARKING
static char *multi_func = "{\n\
  int count_timestamps = 0;\n\
  int event_index = 0;\n\
  static int max_degree = -1;\n\
  static int first_found_index = -1;\n\
  static metrics_t aggregated_event;\n\
  static int current_node_degree = 0;\n\
  int i;\n\
  \n\
  static int pulse_timestamp = 0;\n\
  static int count = 0;\n\
  \n\
  metrics_t *a;\n\
  \n\
start:\n\
  if (first_found_index == -1) {\n\
    count = 0;\n\
    for(i = 0; i < EVcount_metrics_t(); i++) {\n\
      a = EVdata_metrics_t(i);\n\
      if(a->timestamp == pulse_timestamp) {\n\
        int j;\n\
        aggregated_event.metrics_nr = a->metrics_nr;\n\
        for(j = 0; j < a->metrics_nr; j++) {\n\
          aggregated_event.gather_info[j].min = a->gather_info[j].min;\n\
          aggregated_event.gather_info[j].max = a->gather_info[j].max;\n\
          aggregated_event.gather_info[j].sum = a->gather_info[j].sum;\n\
        }\n\
        max_degree = a->max_degree;\n\
        aggregated_event.max_degree = a->max_degree;\n\
        aggregated_event.update_file = a->update_file;\n\
        aggregated_event.timestamp = pulse_timestamp;\n\
        aggregated_event.parent_rank = (a->parent_rank - 1) / max_degree;\n\
        aggregated_event.nprocs = a->nprocs;\n\
        first_found_index = i;\n\
        break;\n\
      }\n\
    }\n\
    current_node_degree = 0;\n\
    for(i = 1; i <= max_degree; ++i) {\n\
      if(a->parent_rank * max_degree + i < a->nprocs) {\n\
        ++current_node_degree;\n\
      }\n\
    }\n\
    \n\
    count = 1;\n\
    EVdiscard_metrics_t(first_found_index);\n\
  } else if (count < current_node_degree + 1) {\n\
    for(i = first_found_index; i < EVcount_metrics_t(); i++) {\n\
      a = EVdata_metrics_t(i);\n\
      if(a->timestamp == pulse_timestamp) {\n\
        int j;\n\
        for(j = 0; j < a->metrics_nr; j++) {\n\
          aggregated_event.gather_info[j].sum += a->gather_info[j].sum;\n\
          if(aggregated_event.gather_info[j].min > a->gather_info[j].min) {\n\
            aggregated_event.gather_info[j].min = a->gather_info[j].min;\n\
          }\n\
          \n\
          if(aggregated_event.gather_info[j].max < a->gather_info[j].max) {\n\
            aggregated_event.gather_info[j].max = a->gather_info[j].max;\n\
          }\n\
        }\n\
        ++count;\n\
        \n\
        first_found_index = i;\n\
        EVdiscard_metrics_t(first_found_index);\n\
        break;\n\
      }\n\
    }\n\
  }\n\
  if (count == current_node_degree + 1) { \n\
    EVsubmit(0, aggregated_event);\n\
    ++pulse_timestamp;\n\
    first_found_index = -1;\n\
    count = 0;\n\
    for(i = 0; i < EVcount_metrics_t(); ++i) {\n\
      a = EVdata_metrics_t(i);\n\
      if(a->timestamp == pulse_timestamp) {\n\
        goto start;\n\
      }\n\
    }\n\
  }\n\
}\0\0";
#else
static char *multi_func = "{\n\
  int count_timestamps = 0;\n\
  int event_index = 0;\n\
  static int max_degree = -1;\n\
  static int first_found_index = -1;\n\
  static metrics_t aggregated_event;\n\
  static int current_node_degree = 0;\n\
  int i;\n\
  \n\
  static int pulse_timestamp = 0;\n\
  static int count = 0;\n\
  \n\
  metrics_t *a;\n\
  \n\
start:\n\
  if (first_found_index == -1) {\n\
    count = 0;\n\
    for(i = 0; i < EVcount_metrics_t(); i++) {\n\
      a = EVdata_metrics_t(i);\n\
      if(a->timestamp == pulse_timestamp) {\n\
        int j;\n\
        aggregated_event.metrics_nr = a->metrics_nr;\n\
        for(j = 0; j < a->metrics_nr; j++) {\n\
          aggregated_event.gather_info[j].min = a->gather_info[j].min;\n\
          aggregated_event.gather_info[j].max = a->gather_info[j].max;\n\
          aggregated_event.gather_info[j].sum = a->gather_info[j].sum;\n\
        }\n\
        max_degree = a->max_degree;\n\
        aggregated_event.max_degree = a->max_degree;\n\
        aggregated_event.update_file = a->update_file;\n\
        aggregated_event.timestamp = pulse_timestamp;\n\
        aggregated_event.parent_rank = (a->parent_rank - 1) / max_degree;\n\
        aggregated_event.nprocs = a->nprocs;\n\
        aggregated_event.start_time = a->start_time;\n\
        first_found_index = i;\n\
        break;\n\
      }\n\
    }\n\
    current_node_degree = 0;\n\
    for(i = 1; i <= max_degree; ++i) {\n\
      if(a->parent_rank * max_degree + i < a->nprocs) {\n\
        ++current_node_degree;\n\
      }\n\
    }\n\
    \n\
    count = 1;\n\
    EVdiscard_metrics_t(first_found_index);\n\
  } else if (count < current_node_degree + 1) {\n\
    for(i = first_found_index; i < EVcount_metrics_t(); i++) {\n\
      a = EVdata_metrics_t(i);\n\
      if(a->timestamp == pulse_timestamp) {\n\
        int j;\n\
        for(j = 0; j < a->metrics_nr; j++) {\n\
          aggregated_event.gather_info[j].sum += a->gather_info[j].sum;\n\
          if(aggregated_event.gather_info[j].min > a->gather_info[j].min) {\n\
            aggregated_event.gather_info[j].min = a->gather_info[j].min;\n\
          }\n\
          \n\
          if(aggregated_event.gather_info[j].max < a->gather_info[j].max) {\n\
            aggregated_event.gather_info[j].max = a->gather_info[j].max;\n\
          }\n\
          if(a->start_time != -1 && a->start_time < aggregated_event.start_time) {\n\
              aggregated_event.start_time = a->start_time;\n\
          }\n\
        }\n\
        ++count;\n\
        \n\
        first_found_index = i;\n\
        EVdiscard_metrics_t(first_found_index);\n\
        break;\n\
      }\n\
    }\n\
  }\n\
  if (count == current_node_degree + 1) { \n\
    // printf(\"%d %d\\n\", aggregated_event->timestamp, EVcount_metrics_t());\n\
    EVsubmit(0, aggregated_event);\n\
    pulse_timestamp = pulse_timestamp + 1;\n\
    first_found_index = -1;\n\
    count = 0;\n\
    for(i = 0; i < EVcount_metrics_t(); ++i) {\n\
      a = EVdata_metrics_t(i);\n\
      if(a->timestamp == pulse_timestamp) {\n\
        goto start;\n\
      }\n\
    }\n\
  }\n\
}\0\0";
#endif

#endif