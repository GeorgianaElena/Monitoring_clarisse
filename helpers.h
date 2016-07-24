#ifndef MPI_FUNC_H
#define MPI_FUNC_H

////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct _metrics_aggregator_t metrics_aggregator_t;

////////////////////////////////////////////////////////////////////////////////////////////////////

int is_leaf(metrics_aggregator_t *aggregator);

/* Get parent rank in current topology */
int get_parent(metrics_aggregator_t *aggregator);

int get_degree_node(metrics_aggregator_t *aggregator);

/* Receive EVPath address of parent split stone through MPI */
void recv_addr_from_parent(metrics_aggregator_t *aggregator, char *addr);

/* Send address to children through MPI */
void send_addr_to_children(metrics_aggregator_t *aggregator, char *addr);

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
