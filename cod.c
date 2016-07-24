  
  int count_timestamps = 0;
  int event_index = 0;
  int max_degree = -1;
  int first_found_index = 0;
  
  static int counter = 0;
  
  metrics_t *a;
  
  /* Find firt metric with the desired timestamp */
  if (EVcount_metrics_t()) {
    int i;
    for(i = 0; i < EVcount_metrics_t(); i++) {
      a = EVdata_metrics_t(i);
      if(a->timestamp == counter) {
        max_degree = a->max_degree;
        first_found_index = i;
        break;
      }
    }
  }
  
  int current_node_degree = 0;
  int i;
  for(i = 1; i <= max_degree; ++i) {
    if(a->parent_rank * max_degree + i < a->nprocs) {
      ++current_node_degree;
    }
  }
  if (EVcount_metrics_t() >= current_node_degree + 1) {
    int i;
    
    metrics_t c;
    c.metrics_nr = a->metrics_nr;
    
    /* Find if there are enough events in the queue with the desired timestamp */
    for(i = 0; i < EVcount_metrics_t(); i++) {
      metrics_t *b = EVdata_metrics_t(i);
      if(b->timestamp == counter) {
        ++count_timestamps;
      }
    }
    
    /*If there are enough events => aggregate them */
    if(count_timestamps == current_node_degree + 1) {
      if(a->timestamp == counter) {
        for(i = 0; i < a->metrics_nr; i++) {
          c.gather_info[i].min = a->gather_info[i].min;
          c.gather_info[i].max = a->gather_info[i].max;
          c.gather_info[i].sum = a->gather_info[i].sum;
        }
        c.max_degree = a->max_degree;
        c.update_file = a->update_file;
        c.timestamp = counter;
        c.parent_rank = (a->parent_rank - 1) / max_degree;
        c.nprocs = a->nprocs;
      }
      
      while(event_index < EVcount_metrics_t()) {
        if(event_index != first_found_index){
          a = EVdata_metrics_t(event_index);
          if(a->timestamp == counter) {
            for(i = 0; i < a->metrics_nr; i++) {
              c.gather_info[i].sum += a->gather_info[i].sum;
              if(c.gather_info[i].min > a->gather_info[i].min) {
                c.gather_info[i].min = a->gather_info[i].min;
              }
              
              if(c.gather_info[i].max < a->gather_info[i].max) {
                c.gather_info[i].max = a->gather_info[i].max;
              }
            }
          }
        }
        event_index++;
      }
      
      /* Discard events already aggregated */
      int i = 0;
      while(i < EVcount_metrics_t()) {
        metrics_t *b = EVdata_metrics_t(i);
        if(b->timestamp == counter) {
          EVdiscard_metrics_t(i);
        } else {
          i++;
        }
      }
      /* submit the new, combined event */
      ++counter;
      EVsubmit(0, c);
    }
  }
}