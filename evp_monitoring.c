#include "evp_monitoring.h"
#include "helpers.h"
#include "metrics_crawler.h"

#include "mpi.h"
#include "string.h"
#include "stdio.h"
#include "metrics_aggregator.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef BENCHMARKING
#define MAX_FILENAME_LENGTH 100
#endif

#define ADDRESS_SIZE 2048
#define ROOT 0

////////////////////////////////////////////////////////////////////////////////////////////////////

void monitoring_initialize(evp_monitoring_t *mon, char *a_f,  uint64_t ts_nr,
                           int d, unsigned int pulse)
{
  int rank, nprocs;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

  mon->aggregator = malloc(sizeof(metrics_aggregator_t));

  initialize_metrics_aggregator(mon->aggregator, a_f, ts_nr, d, pulse);

/* Create the directory and file to hold benchmark results */
#ifdef BENCHMARKING
  if(rank == ROOT) {
    char filename[MAX_FILENAME_LENGTH];
    char dirname[MAX_FILENAME_LENGTH];
    long nr_metrics;

    mon->aggregator->metrics_file = fopen (mon->aggregator->aliases_file, "r");
    if(!mon->aggregator->metrics_file) {
      perror("Error on opening metric aliases file");
      exit(-1);
    }

    if(fscanf(mon->aggregator->metrics_file, "%ld", &nr_metrics) == 0) {
      perror("Error on reading metrics file");
      exit(-1);
    }

    fclose(mon->aggregator->metrics_file);

    sprintf(dirname, "Monitoring_results");

    struct stat st = { 0 };
    if (stat(dirname, &st) == -1) {
      mkdir(dirname, S_IRWXU);
    }

    sprintf(filename, "./%s/evpath_%dnodes_%ddegree_%ldevents_%ldmetrics_%upulse",
                       dirname, nprocs, mon->aggregator->max_degree, mon->aggregator->max_timestamps,
                       nr_metrics, mon->aggregator->pulse_interval);

    mon->aggregator->results = fopen(filename, "w");

    if(!mon->aggregator->results) {
      perror("Error oppening benchmarking results file");
      exit(-1);
    }
  }
#endif

  initialize_metrics_crawler(mon->aggregator);

  /* Receive evpath address from parent node */
  if(rank != ROOT) {
    char addr[ADDRESS_SIZE];
    recv_addr_from_parent(mon->aggregator, addr);
    memcpy(mon->aggregator->current_state.parent_addr, addr, ADDRESS_SIZE);
  }

  /* Compute evpath address and send to children */
  if(!is_leaf(mon->aggregator)) {
    char myaddr[ADDRESS_SIZE];
    compute_evpath_addr(mon->aggregator, myaddr);
    send_addr_to_children(mon->aggregator, myaddr);
  }

  /* Wait for all the processes to finish init phase */
  MPI_Barrier(MPI_COMM_WORLD);

  /* Fork a thread to handle the network input operations of a connection manager */
  if(!is_leaf(mon->aggregator)) {
    if(!CMfork_comm_thread(mon->aggregator->current_state.conn_mgr)) {
      fprintf(stderr, "Communications manager thread could not be forked\n");
    }
  }

  /* Create EVPath stones and set actions */
  set_stones_actions(mon->aggregator);

  /* Wait for all the processes to setup stones */
  MPI_Barrier(MPI_COMM_WORLD);

  MPI_Comm_split(MPI_COMM_WORLD, is_leaf(mon->aggregator), rank,
                 &mon->aggregator->comm_leafs);

  /* Start aggregating metrics off the main thread*/
  if (is_leaf(mon->aggregator)) {
    start_leaf_thread(mon->aggregator);
  }
}

////////////////////////////////////////////////////////////////////////////////

void monitoring_finalize(evp_monitoring_t *mon)
{
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

#ifndef BENCHMARKING
  if (is_leaf(mon->aggregator)) {
    pthread_mutex_lock(&mon->aggregator->glock);
    mon->aggregator->leafs_finished = 1;
    pthread_mutex_unlock(&mon->aggregator->glock);
  }
#endif

  if (rank == ROOT) {
    pthread_mutex_lock(&mon->aggregator->glock);
    while (!mon->aggregator->root_finished) {
      pthread_cond_wait(&mon->aggregator->cond_root_finished, &mon->aggregator->glock);
    }
    pthread_mutex_unlock(&mon->aggregator->glock);
  }

  /* Wait for all the nodes to finish*/

  MPI_Barrier(MPI_COMM_WORLD);

  MPI_Comm_free(&mon->aggregator->comm_leafs);

  destroy_metrics_aggregator(mon->aggregator);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef BENCHMARKING
void return_results(evp_monitoring_t *mon, sys_metric_t *results)
{
  static int init = 1;

  pthread_mutex_lock(&mon->aggregator->results_lock);
  if (init) {
    while(!mon->aggregator->results_available) {
      pthread_cond_wait(&mon->aggregator->results_cond, &mon->aggregator->results_lock);
    }
  }

  for(int i = 0; i < mon->aggregator->total_metrics; ++i) {
    results[i].min = mon->aggregator->monitoring_results[i].min;
    results[i].max = mon->aggregator->monitoring_results[i].max;
    results[i].avg = mon->aggregator->monitoring_results[i].avg;
  }

  if (init) {
    mon->aggregator->results_available = 0;
    init = 0;
  }
  pthread_mutex_unlock(&mon->aggregator->results_lock);
}

void monitoring_file_updated(evp_monitoring_t *mon)
{
  pthread_mutex_lock(&mon->aggregator->results_lock);
  while(!mon->aggregator->results_available) {
    pthread_cond_wait(&mon->aggregator->results_cond, &mon->aggregator->results_lock);
  }

  mon->aggregator->results_available = 0;
  pthread_mutex_unlock(&mon->aggregator->results_lock);
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////