#ifndef STORAGE_H
#define STORAGE_H

#include "available_metrics.h"
#include "string.h"
#include "stdio.h"
#include "pthread.h"

typedef double (*func_ptr)();

/* Initialize htable with the metrics known by the system */
static int init()
{
  int retval;

  retval = PAPI_library_init(PAPI_VER_CURRENT);
  if (retval != PAPI_VER_CURRENT) {
    fprintf(stderr, "Error! PAPI_library_init %d\n",retval);
    goto cleanup;
  }

  return 0;
}

/* Return function associated with metric alias */
static func_ptr get_value(char *key)
{
  htable *pair = NULL;

  HASH_FIND_STR(callbacks_storage, key, pair);

  if (!pair) {
    return NULL;
  }

  return pair->value;
}

#endif