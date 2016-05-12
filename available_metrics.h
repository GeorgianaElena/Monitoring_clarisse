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
    fprintf(stderr, "%s\n", "Failed reading /proc/uptime");
  }

  fclose(procuptime);

  return (sec*tickspersec)+ssec;
}

static double get_cpu_utilization()
{
  FILE *proc;

  char c;

  int i = 0, cpu_found = 0, tmp = 0, total = 0;
  int diff_idle = 0, diff_total = 0;
  int idle = 0, x = 0, diff_usage = 0;

  if((proc = fopen("/proc/stat","r")) == NULL) {
    printf("Failed to open /proc/stat\n");

    return 1;
  }

  while((c = fgetc(proc)) != EOF ) {
    if( (c == 'c' && i == 0) || (c == 'p' && i == 1) || (c == 'u' && i == 2)) {
      cpu_found++;
    } else if(c >= '0' && c <= '9' && cpu_found == 3) {
      tmp *= 10;
      tmp += c - '0';
    } else if(c == ' ' && cpu_found == 3 ) {
      total += tmp;
      x++;

      if(x == 6) {
        idle = tmp;
      }

      tmp = 0;
    } else if(c == '\n' && cpu_found == 3) {
      i = 0;
      cpu_found = 0;
      total += tmp;
      tmp = 0;

      break;
    }

    i++;
  }

  diff_idle = idle;
  diff_total = total;

  diff_usage = 100 * (diff_total - diff_idle);

  fclose(proc);

  return (float)diff_usage / (float)diff_total;
}

static double get_memory_utilization()
{
  double tSize = 0, resident = 0, share = 0;
  FILE *buffer = fopen("/proc/self/statm","r");

  if(!fscanf(buffer, "%lf\n%lf\n%lf\n", &tSize, &resident, &share)) {
    fprintf(stderr, "%s\n", "Failed reading /proc/self/statm");
  }

  fclose(buffer);

  long page_size_kb = sysconf(_SC_PAGE_SIZE) / 1024; // in case x86-64 is configured to use 2MB pages

  // double rss = resident * page_size_kb;
  // cout << "RSS - " << rss << " kB\n";

  double shared_mem = share * page_size_kb;
  // cout << "Shared Memory - " << shared_mem << " kB\n";

  // cout << "Private Memory - " << rss - shared_mem << "kB\n";

  return shared_mem;
}


/*------------------------------------
   Populate storage with known metrics
  ------------------------------------
*/
static int available_metrics_no = 4;

static const callback_t callbacks[] = 
{
  {return_rank, "rank_number"},
  {get_uptime, "uptime"},
  {get_cpu_utilization, "cpu_utilization"},
  {get_memory_utilization, "memory_utilization"}
};

#endif