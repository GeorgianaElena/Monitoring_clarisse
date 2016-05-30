#ifndef METRICS_CRAWLER_H
#define METRICS_CRAWLER_H

#include "available_metrics.h"
#include "stdio.h"

static FILE *metrics_file;

static char **desired_metrics = NULL;

// static int *Events = NULL;

static long total_metrics = 0;

static long old_nr_of_metrics = 0;

typedef struct _aggregators_t aggregators_t;

void initialize_metrics_crawler()
{
  int retval;

  retval = PAPI_library_init(PAPI_VER_CURRENT);
  if (retval != PAPI_VER_CURRENT) {
    fprintf(stderr, "Error! PAPI_library_init %d\n",retval);
  }
}

void initialize_metrics_crawler_number_from_file(long *nr, char* filename)
{
  metrics_file = fopen (filename, "r");
  if(!metrics_file) {
    perror("Error on opening metric aliases file");
    exit(-1);
  }

  if(fscanf(metrics_file, "%ld", nr) == 0) {
    perror("Error on reading metrics file");
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
    // free(Events);

    desired_metrics = NULL;
    // Events = NULL;
  }

  desired_metrics = (char **) calloc (total_metrics, sizeof(char *));
  // printf("am alocat %ld \n", total_metrics);
  // Events = (int *) calloc (total_metrics, sizeof(int));
}

void initialize_metrics_crawler_number_from_memory(long *nr)
{
  *nr = total_metrics;
}

void metrics_crawler_results_memory(aggregators_t *result)
{
  compute_metric(desired_metrics, result, total_metrics, 0);
}

int metrics_crawler_results_file(aggregators_t *result, char *filename)
{
  metrics_file = fopen(filename, "r");
  if(!metrics_file) {
    perror("Error on opening metric aliases file");
    exit(-1);
  }

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
    // printf("%ld\n", metric_number);
    if((number != old_nr_of_metrics) ||
       (desired_metrics[metric_number] && strcmp(desired_metrics[metric_number], line))) {
      needs_sync = true;
    }

    desired_metrics[metric_number] = strdup(line);

    ++metric_number;
  }

  compute_metric(desired_metrics, result, total_metrics, 1);

  fclose(metrics_file);

  return needs_sync;
}

#endif