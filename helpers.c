#include "helpers.h"
#include "metrics_aggregator.h"

#include "mpi.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

#define ADDRESS_SIZE 2048

////////////////////////////////////////////////////////////////////////////////////////////////////

int is_leaf(metrics_aggregator_t *aggregator)
{
  int rank;
  int nprocs;

  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  return ((rank * aggregator->max_degree + 1 >= nprocs) && (nprocs > 1)); 
}

////////////////////////////////////////////////////////////////////////////////////////////////////

/* Get parent rank in current topology */
int get_parent(metrics_aggregator_t *aggregator)
{
  int rank;
  int nprocs;

  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  return (rank - 1) / aggregator->max_degree;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int get_degree_node(metrics_aggregator_t *aggregator)
{
  int rank;
  int nprocs;

  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  int degree = 0;

  for(int i = 1; i <= aggregator->max_degree; ++i) {
    if(rank * aggregator->max_degree + i < nprocs) {
      ++degree;
    }
  }

  return degree;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

/* Receive EVPath address of parent split stone through MPI */
void recv_addr_from_parent(metrics_aggregator_t *aggregator, char *addr)
{
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  MPI_Recv(addr, ADDRESS_SIZE, MPI_CHAR, get_parent(aggregator), 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

/* Send address to children through MPI */
void send_addr_to_children(metrics_aggregator_t *aggregator, char *addr)
{
  int rank, tag = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  for(int i = 1; i <= get_degree_node(aggregator); ++i) {
    MPI_Send(addr, ADDRESS_SIZE, MPI_CHAR, rank * aggregator->max_degree + i, tag, MPI_COMM_WORLD);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////