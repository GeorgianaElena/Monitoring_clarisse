#ifndef METRICS_CRAWLER_H
#define METRICS_CRAWLER_H

#include "available_metrics.h"
#include "storage.h"
#include "stdio.h"
#include "stdbool.h"

static FILE *metrics_file;

static char **desired_metrics = NULL;

static long total_metrics = 0;

static long old_nr_of_metrics = 0;

typedef struct _aggregators_t aggregators_t;

void initialize_metrics_crawler()
{
  init();
}

void initialize_metrics_crawler_number_from_file(long *nr, char* filename)
{
  metrics_file = fopen (filename, "r");

  if(fscanf(metrics_file, "%ld", nr) == 0) {
    fprintf(stderr, "%s\n", "Error on reading metrics file");
    exit(-1);
  }

  fclose(metrics_file);

  old_nr_of_metrics = total_metrics;

  total_metrics = *nr;

  if(desired_metrics) {

    for(int i = 0; i < old_nr_of_metrics; ++i) {
        free(desired_metrics[i]);
        desired_metrics[i] = NULL;
    }

    free(desired_metrics);

    desired_metrics = NULL;
  }

  desired_metrics = (char **) calloc (total_metrics, sizeof(char *));
}

void initialize_metrics_crawler_number_from_memory(long *nr)
{
  *nr = total_metrics;
}

void metrics_crawler_results_memory(aggregators_t *result)
{
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  for(int i = 0; i < total_metrics; ++i) {
    double (*func)();

    func = get_value(desired_metrics[i]);

    double metric_result = 0;
    
    if(!func && !rank) {
        fprintf(stderr, "Metric \"%s\" not supported\n", desired_metrics[i]);
    } else if(func){
        metric_result = func();
    }

    result[i].min = metric_result;
    result[i].max = metric_result;
    result[i].sum = metric_result;
  }
}

int metrics_crawler_results_file(aggregators_t *result, char *filename)
{
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  metrics_file = fopen(filename, "r");

  char line[MAX_LENGTH_ALIAS];

  long metric_number = 0;

  long number = 0;

  int needs_sync = false;

  if (metrics_file == NULL) {
    exit(-1);
  }

  if(fscanf(metrics_file, "%ld\n", &number) == -1) {
    fprintf(stderr, "%s\n", "Error while reading metrics file");
    exit(-1);
  }

  while (fscanf(metrics_file, "%s\n", line) != -1 && metric_number < total_metrics) {
    if((number != old_nr_of_metrics) ||
       (desired_metrics[metric_number] && strcmp(desired_metrics[metric_number], line))) {
      needs_sync = true;
    }

    desired_metrics[metric_number] = strdup(line);

    double (*func)();

    func = get_value(line);

    double metric_result = 0;

    if(!func && !rank) {
      fprintf(stderr, "Metric \"%s\" not supported\n", line);
    } else if(func) {
      metric_result = func();
    }


    result[metric_number].min = metric_result;
    result[metric_number].max = metric_result;
    result[metric_number].sum = metric_result;

    ++metric_number;
  }

  fclose(metrics_file);

  return needs_sync;
}

#endif