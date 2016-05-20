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
// #define BENCHMARKING

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
  static int counter = 0;\n\
  static int count = 0;\n\
  \n\
  metrics_t *a;\n\
  \n\
  if (first_found_index == -1) {\n\
    count = 0;\n\
    for(i = 0; i < EVcount_metrics_t(); i++) {\n\
      a = EVdata_metrics_t(i);\n\
      if(a->timestamp == counter) {\n\
        int j;\n\
        // printf(\"sunt: %d si am primit eventul 0\\n\", a->parent_rank);\n\
        aggregated_event.metrics_nr = a->metrics_nr;\n\
        for(j = 0; j < a->metrics_nr; j++) {\n\
          // printf(\"sunt %d si primesc %lf\\n\", a->parent_rank, a->gather_info[j].min);\n\
          aggregated_event.gather_info[j].min = a->gather_info[j].min;\n\
          aggregated_event.gather_info[j].max = a->gather_info[j].max;\n\
          aggregated_event.gather_info[j].sum = a->gather_info[j].sum;\n\
        }\n\
        max_degree = a->max_degree;\n\
        aggregated_event.max_degree = a->max_degree;\n\
        aggregated_event.update_file = a->update_file;\n\
        aggregated_event.timestamp = counter;\n\
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
      if(a->timestamp == counter) {\n\
        int j;\n\
        for(j = 0; j < a->metrics_nr; j++) {\n\
          // printf(\"sunt %d si primesc %lf\\n\", a->parent_rank, a->gather_info[j].min);\n\
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
    ++counter;\n\
    first_found_index = -1;\n\
    count = 0;\n\
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
  static int counter = 0;\n\
  static int count = 0;\n\
  \n\
  metrics_t *a;\n\
  \n\
  if (first_found_index == -1) {\n\
    count = 0;\n\
    for(i = 0; i < EVcount_metrics_t(); i++) {\n\
      a = EVdata_metrics_t(i);\n\
      if(a->timestamp == counter) {\n\
        int j;\n\
        // printf(\"sunt: %d si am primit eventul 0\\n\", a->parent_rank);\n\
        aggregated_event.metrics_nr = a->metrics_nr;\n\
        for(j = 0; j < a->metrics_nr; j++) {\n\
          // printf(\"sunt %d si primesc %lf\\n\", a->parent_rank, a->gather_info[j].min);\n\
          aggregated_event.gather_info[j].min = a->gather_info[j].min;\n\
          aggregated_event.gather_info[j].max = a->gather_info[j].max;\n\
          aggregated_event.gather_info[j].sum = a->gather_info[j].sum;\n\
        }\n\
        max_degree = a->max_degree;\n\
        aggregated_event.max_degree = a->max_degree;\n\
        aggregated_event.update_file = a->update_file;\n\
        aggregated_event.timestamp = counter;\n\
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
      if(a->timestamp == counter) {\n\
        int j;\n\
        for(j = 0; j < a->metrics_nr; j++) {\n\
          // printf(\"sunt %d si primesc %lf\\n\", a->parent_rank, a->gather_info[j].min);\n\
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
    EVsubmit(0, aggregated_event);\n\
    ++counter;\n\
    first_found_index = -1;\n\
    count = 0;\n\
  }\n\
}\0\0";
//   printf(\"In coada = %d\\n\", EVcount_metrics_t());\n\
//   int count_timestamps = 0;\n\
//   int event_index = 0;\n\
//   int max_degree = -1;\n\
//   int first_found_index = 0;\n\
//   \n\
//   static int counter = 0;\n\
//   \n\
//   metrics_t *a;\n\
//   \n\
//   /* Find firt metric with the desired timestamp */\n\
//   if (EVcount_metrics_t()) {\n\
//     int i;\n\
//     for(i = 0; i < EVcount_metrics_t(); i++) {\n\
//       a = EVdata_metrics_t(i);\n\
//       if(a->timestamp == counter) {\n\
//         max_degree = a->max_degree;\n\
//         first_found_index = i;\n\
//         break;\n\
//       }\n\
//     }\n\
//   }\n\
//   \n\
//   int current_node_degree = 0;\n\
//   int i;\n\
//   for(i = 1; i <= max_degree; ++i) {\n\
//     if(a->parent_rank * max_degree + i < a->nprocs) {\n\
//       ++current_node_degree;\n\
//     }\n\
//   }\n\
//   if (EVcount_metrics_t() >= current_node_degree + 1) {\n\
//     int i;\n\
//     \n\
//     metrics_t c;\n\
//     c.metrics_nr = a->metrics_nr;\n\
//     \n\
//     /* Find if there are enough events in the queue with the desired timestamp */\n\
//     for(i = 0; i < EVcount_metrics_t(); i++) {\n\
//       metrics_t *b = EVdata_metrics_t(i);\n\
//       if(b->timestamp == counter) {\n\
//         ++count_timestamps;\n\
//       }\n\
//     }\n\
//     \n\
//     /*If there are enough events => aggregate them */\n\
//     if(count_timestamps == current_node_degree + 1) {\n\
//       if(a->timestamp == counter) {\n\
//         c.start_time = a->start_time;\n\
//         for(i = 0; i < a->metrics_nr; i++) {\n\
//           c.gather_info[i].min = a->gather_info[i].min;\n\
//           c.gather_info[i].max = a->gather_info[i].max;\n\
//           c.gather_info[i].sum = a->gather_info[i].sum;\n\
//         }\n\
//         c.max_degree = a->max_degree;\n\
//         c.update_file = a->update_file;\n\
//         c.timestamp = counter;\n\
//         c.parent_rank = (a->parent_rank - 1) / max_degree;\n\
//         c.nprocs = a->nprocs;\n\
//       }\n\
//       \n\
//       while(event_index < EVcount_metrics_t()) {\n\
//         if(event_index != first_found_index){\n\
//           a = EVdata_metrics_t(event_index);\n\
//           if(a->timestamp == counter) {\n\
//             for(i = 0; i < a->metrics_nr; i++) {\n\
//               c.gather_info[i].sum += a->gather_info[i].sum;\n\
//               if(c.gather_info[i].min > a->gather_info[i].min) {\n\
//                 c.gather_info[i].min = a->gather_info[i].min;\n\
//               }\n\
//               \n\
//               if(c.gather_info[i].max < a->gather_info[i].max) {\n\
//                 c.gather_info[i].max = a->gather_info[i].max;\n\
//               }\n\
//             }\n\
//             if(a->start_time != -1 && a->start_time < c.start_time) {\n\
//               c.start_time = a->start_time;\n\
//             }\n\
//           }\n\
//         }\n\
//         event_index++;\n\
//       }\n\
//       \n\
//       /* Discard events already aggregated */\n\
//       int i = 0;\n\
//       while(i < EVcount_metrics_t()) {\n\
//         metrics_t *b = EVdata_metrics_t(i);\n\
//         if(b->timestamp == counter) {\n\
//           EVdiscard_metrics_t(i);\n\
//         } else {\n\
//           i++;\n\
//         }\n\
//       }\n\
//       /* submit the new, combined event */\n\
//       ++counter;\n\
//       EVsubmit(0, c);\n\
//     }\n\
//   }\n\
// }\0\0";

#endif

#endif