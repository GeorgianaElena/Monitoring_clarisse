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

typedef struct {
	char parent_addr[ADDRESS_SIZE];
	EVstone multi_stone;
	EVstone bridge_stone;

	//valid only for root of the tree
	EVstone terminal_stone;
	CManager conn_mgr;
} node_state_t;

typedef struct {
  int integer_field;
  int degree;
} metrics_t, *metrics_t_ptr;


static FMField metrics_field_list[] =
{
  {"integer_field", "integer", sizeof(int), FMOffset(metrics_t_ptr, integer_field)},
  {"degree", "integer", sizeof(int), FMOffset(metrics_t_ptr, degree)},
  {NULL, NULL, 0, 0}
};

static FMStructDescRec metrics_format_list[] =
{
  {"metrics_t", metrics_field_list, sizeof(metrics_t), NULL},
  {NULL, NULL}
};

static FMStructDescList queue_list[] = {metrics_format_list, NULL};

static int final_result(CManager cm, void *vevent, void *client_data, attr_list attrs);
static int is_leaf();
static int get_parent();

void recv_addr_from_parent(char *addr);
void send_addr_to_parent(char *addr);
void initialize_monitoring();
void create_stones();
void start_listening();
void start_communication();

void compute_evpath_addr(char *addr);

#endif