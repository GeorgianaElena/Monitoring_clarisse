#ifndef METRICS_AGGREGATOR_H
#define METRICS_AGGREGATOR_H

#include "mpi.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "sys/stat.h"
#include "sys/types.h"
#include "fcntl.h"
#include "unistd.h"
#include "evpath.h"

#define DEGREE 2
#define ADDRESS_SIZE 2048

typedef struct _simple_rec {
  int integer_field;
} simple_rec, *simple_rec_ptr;

static FMField simple_field_list[] =
{
  {"integer_field", "integer", sizeof(int), FMOffset(simple_rec_ptr, integer_field)},
  {NULL, NULL, 0, 0}
};

static FMStructDescRec simple_format_list[] =
{
  {"simple", simple_field_list, sizeof(simple_rec), NULL},
  {NULL, NULL}
};

static int simple_handler(CManager cm, void *vevent, void *client_data, attr_list attrs);
static int is_leaf();
static int get_parent();

void recv_addr_from_parent(char *addr);
void send_addr_to_parent(char *addr);
void initialize_monitoring(char* parents_addr[]);

CManager compute_evpath_addr(char *addr);

#endif