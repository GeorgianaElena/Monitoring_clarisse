#ifndef AVAILABLE_METRICS_H
#define AVAILABLE_METRICS_H

#define MAX_LENGTH_ALIAS 100

typedef struct _callback_t {
  long (*func)();
  char alias[MAX_LENGTH_ALIAS];
} callback_t;

/*------------------------------------
  Implement new metrics here
  ------------------------------------
*/
long return_rank();

long get_uptime();

long benchmark_metric();

/*------------------------------------
   Populate storage with metric abov
  ------------------------------------
*/
int available_metrics_no = 3;

const callback_t callbacks[] = 
{
  {return_rank,      "rank"},
  {get_uptime,       "uptime"},
  {benchmark_metric, "test"}
};

#endif