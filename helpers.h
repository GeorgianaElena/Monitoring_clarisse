#ifndef HELPERS_H
#define HELPERS_H

#define BENCHMARKING

MPI_Comm comm_leafs;
MPI_Group group_leafs, grp_world;

#ifdef BENCHMARKING
static void create_leafs_comm()
{
  int nprocs, j = 0, i;
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

  int *not_leafs;
  not_leafs = calloc(nprocs, sizeof(int));

  for (i = 1; i < nprocs; ++i) {
    if(!(i * DEGREE + 1 >= nprocs) && (nprocs > 1)) {
      not_leafs[j] = i;
      ++j;
    }
  }

  MPI_Comm_group(MPI_COMM_WORLD, &grp_world);
  MPI_Group_excl(grp_world, j, not_leafs, &group_leafs);
  MPI_Comm_create(MPI_COMM_WORLD, group_leafs, &comm_leafs);
}

static void measure_time(double *curr_time, double *mintime)
{
  *curr_time = MPI_Wtime();
  MPI_Reduce(curr_time, mintime, 1, MPI_DOUBLE, MPI_MIN, 0, comm_leafs);
}
#endif

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