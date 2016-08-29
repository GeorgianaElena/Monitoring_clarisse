#ifndef COD_H
#define COD_H

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
  int cnt = EVcount_metrics_t();\n\
  static int flag = 0;\n\
  metrics_t *b = EVdata_metrics_t(cnt - 1);\n\
  if(b.quit == 1) {\n\
    flag += 1;\n\
  }\n\
start:\n\
  if (first_found_index == -1) {\n\
    count = 0;\n\
    for(i = 0; i < cnt; i++) {\n\
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
        aggregated_event.quit = a->quit;\n\
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
    for(i = first_found_index; i < cnt; i++) {\n\
      a = EVdata_metrics_t(i);\n\
      if(a->timestamp == pulse_timestamp) {\n\
        int j;\n\
        for(j = 0; j < a->metrics_nr; j++) {\n\
          aggregated_event.gather_info[j].sum += a->gather_info[j].sum;\n\
          if(aggregated_event.quit < a->quit) {\n\
            aggregated_event.quit = a->quit;\n\
          }\n\
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
  }\n\
  if(flag == current_node_degree + 1) {\n\
    while(EVcount_metrics_t()) {\n\
      goto start;\n\
    }\n\
  }\n\
}\0\0";
////////////////////////////////////////////////////////////////////////////////////////////////////
#else
////////////////////////////////////////////////////////////////////////////////////////////////////
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
  int check = 0;\n\
  int cnt = EVcount_metrics_t();\n\
  static int flag = 0;\n\
  metrics_t *b = EVdata_metrics_t(cnt - 1);\n\
  if(b.quit == 1) {\n\
    flag += 1;\n\
  }\n\
start:\n\
  if (first_found_index == -1) {\n\
    count = 0;\n\
    for(i = 0; i < cnt; i++) {\n\
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
        aggregated_event.quit = a->quit;\n\
        first_found_index = i;\n\
        check = 1;\n\
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
    for(i = first_found_index; i < cnt; i++) {\n\
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
          if(a->start_time != -1 && (aggregated_event.start_time - a->start_time) > 0.0) {\n\
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
    pulse_timestamp = pulse_timestamp + 1;\n\
    first_found_index = -1;\n\
    count = 0;\n\
  }\n\
  if(flag == current_node_degree + 1) {\n\
    while(EVcount_metrics_t()) {\n\
      goto start;\n\
    }\n\
  }\n\
}\0\0";
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
