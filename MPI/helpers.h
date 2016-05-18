#ifndef HELPERS_H
#define HELPERS_H

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

#endif