#ifndef AVAILABLE_METRICS_H
#define AVAILABLE_METRICS_H

#include "mpi.h"

#define MAX_LENGTH_ALIAS 100

typedef struct _callback_t {
  double (*func)();
  char alias[MAX_LENGTH_ALIAS];
} callback_t;


/*------------------------------------
  Implement new metrics here
  ------------------------------------
*/
static double return_rank()
{
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  return rank;
}

static double get_uptime()
{
  FILE *procuptime;
  int sec, ssec;
  long tickspersec = sysconf(_SC_CLK_TCK);

  procuptime = fopen("/proc/uptime", "r");

  if(fscanf(procuptime, "%d.%d", &sec, &ssec) == 0) {
    fprintf(stderr, "%s\n", "Error reading /proc/uptime");
  }

  fclose(procuptime);

  return (sec*tickspersec)+ssec;
}


/*------------------------------------
   Populate storage with known metrics
  ------------------------------------
*/
static int available_metrics_no = 2;

static const callback_t callbacks[] = 
{
  {return_rank, "rank_number"},
  {get_uptime, "uptime"}
};

#endif