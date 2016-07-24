#ifndef STORAGE_H
#define STORAGE_H

#include "string.h"
#include "stdio.h"
#include "pthread.h"
#include "uthash.h"

#define MAX_LENGTH_ALIAS 100

typedef struct htable{
    char key[MAX_LENGTH_ALIAS];
    long (*value)();
    UT_hash_handle hh; /* makes this structure hashable */
} htable;

typedef struct _callback_t {
  long (*func)();
  char alias[MAX_LENGTH_ALIAS];
} callback_t;

typedef long (*func_ptr)();

/* Initialize htable with the metrics known by the system */
int init_known_metrics();

/* Return function associated with metric alias */
func_ptr get_value(char *key);

/*------------------------------------
  Implement new metrics here
  ------------------------------------
*/
long return_rank();

long get_uptime();

long benchmark_metric();

#endif