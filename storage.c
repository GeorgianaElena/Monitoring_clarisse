#include "storage.h"

#include "uthash.h"
#include "papi.h"
#include "mpi.h"
#include "sys/user.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

const callback_t callbacks[] = 
{
  {return_rank,      "rank"  },
  {get_uptime,       "uptime"},
  {benchmark_metric, "test"  }
};

int available_metrics_no = 3;

/* Storage for alias - custom functions pairs*/
htable *callbacks_storage = NULL;

////////////////////////////////////////////////////////////////////////////////////////////////////

/* Initialize htable with the metrics known by the system */
int init_known_metrics()
{
  /* Initialize PAPI library */
  int retval;

  retval = PAPI_library_init(PAPI_VER_CURRENT);
  if (retval != PAPI_VER_CURRENT) {
    fprintf(stderr, "Error! PAPI_library_init %d\n",retval);

    PAPI_shutdown();
  }

  /* Initialize custom metrics storage */
  HASH_CLEAR(hh, callbacks_storage);

  for(int i = 0; i < available_metrics_no; ++i) {
    htable *new_pair = NULL;
    new_pair = malloc(sizeof(htable));

    if (!new_pair) {
        fprintf(stderr, "can't alloc memory for the new pair\n");
        exit(-1);
    }

    strcpy(new_pair->key, callbacks[i].alias);
    new_pair->value = callbacks[i].func;

    /* insert the new pair in callbacks_storage */
    HASH_ADD_STR(callbacks_storage, key, new_pair);
  }

  return 0;
}

void free_storage()
{
  htable *tmp;
  htable *curr;
  HASH_ITER(hh, callbacks_storage, curr, tmp) {
    HASH_DEL(callbacks_storage, curr);
    free(curr);
  }
}

/* Return function associated with metric alias */
func_ptr get_value(char *key)
{
  htable *pair = NULL;

  HASH_FIND_STR(callbacks_storage, key, pair);

  if (!pair) {
      return(NULL);
  }

  return pair->value;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

/*------------------------------------
  Implement new metrics here
  ------------------------------------
*/
////////////////////////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////////////////////////