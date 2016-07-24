#include "available_metrics.h"

#include "papi.h"
#include "mpi.h"
#include "stdio.h"
#include "sys/user.h"

/*------------------------------------
  Implement new metrics here
  ------------------------------------
*/

long return_rank()
{
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  return rank;
}

long get_uptime()
{
  FILE *procuptime;
  int sec, ssec;
  long tickspersec = sysconf(_SC_CLK_TCK);

  procuptime = fopen("/proc/uptime", "r");

  if(fscanf(procuptime, "%d.%d", &sec, &ssec) == 0) {
    fprintf(stderr, "%s\n", "Failed reading /proc/uptime");
  }

  fclose(procuptime);

  return (sec*tickspersec)+ssec;
}

long benchmark_metric()
{
  return 1;
}
