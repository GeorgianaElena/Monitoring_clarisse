#ifndef AVAILABLE_METRICS_H
#define AVAILABLE_METRICS_H

#include "mpi.h"

#define MAX_LENGTH_ALIAS 100

typedef struct _callback_t {
    double (*func)();
	char alias[MAX_LENGTH_ALIAS];
} callback_t;

static double return_rank()
{
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	return rank;
}

static int available_metrics_no = 1;

static const callback_t callbacks[] = 
{
	{return_rank, "rank number"}
};

#endif