#ifndef MPI_FUNC_H
#define MPI_FUNC_H

MPI_Comm comm_leafs;
MPI_Group group_leafs, grp_world;

static int is_leaf()
{
  int rank;
  int nprocs;

  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  return ((rank * DEGREE + 1 >= nprocs) && (nprocs > 1)); 
}

/* Get parent rank in current topology */
static int get_parent()
{
  int rank;
  int nprocs;

  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  return (rank - 1) / DEGREE;
}

static int get_degree_node()
{
  int rank;
  int nprocs;

  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  int degree = 0;

  for(int i = 1; i <= DEGREE; ++i) {
    if(rank * DEGREE + i < nprocs) {
      ++degree;
    }
  }

  return degree;
}

/* Receive EVPath address of parent split stone through MPI */
void recv_addr_from_parent(char *addr)
{
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  MPI_Recv(addr, ADDRESS_SIZE, MPI_CHAR, get_parent(), 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

  printf("Process %d received from parent %s\n", rank, addr);
}

/* Send address to children through MPI */
void send_addr_to_children(char *addr)
{
  int rank, tag = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  for(int i = 1; i <= get_degree_node(); ++i) {
    MPI_Send(addr, ADDRESS_SIZE, MPI_CHAR, rank * DEGREE + i, tag, MPI_COMM_WORLD);
  }
}
#endif