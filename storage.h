#ifndef STORAGE_H
#define STORAGE_H

#include "available_metrics.h"
#include "uthash.h"
#include "string.h"
#include "stdio.h"
#include "pthread.h"

typedef struct htable{
    char key[MAX_LENGTH_ALIAS];
    double (*value)();
    UT_hash_handle hh; /* Makes this structure hashable */
} htable;

/* Storage for alias-functions pairs*/
static htable *callbacks_storage = NULL;

typedef double (*func_ptr)();

static int init()
{
    HASH_CLEAR(hh, callbacks_storage);
    
    for(int i = 0; i < available_metrics_no; ++i) {
        htable *new_pair = NULL;
        new_pair = malloc(sizeof(htable));

        if (!new_pair) {
            fprintf(stderr, "Cannnot allocate memory for the new pair\n");
            exit(-1);
        }   

        strcpy(new_pair->key, callbacks[i].alias);
        new_pair->value = callbacks[i].func;

        /* Insert the new pair in callbacks_storage */
        HASH_ADD_STR(callbacks_storage, key, new_pair);
    }

    return 0;
}

static func_ptr get_value(char *key)
{
    htable *pair = NULL;

    HASH_FIND_STR(callbacks_storage, key, pair);

    if (!pair) {  
        return NULL;
    }

    /* Copy the value asociated with the desired key */
    return pair->value;
}
#endif