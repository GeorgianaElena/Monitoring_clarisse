#include "metrics_crawler.h"
#include "metrics_aggregator.h"
#include "metric_type.h"
#include "storage.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

void initialize_metrics_crawler()
{
  init_known_metrics();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void initialize_metrics_crawler_number_from_file(metrics_aggregator_t *aggregator, long *nr, char* filename)
{
  aggregator->metrics_file = fopen (filename, "r");
  if(!aggregator->metrics_file) {
    perror("Error on opening metric aliases file");
    exit(-1);
  }

  if(fscanf(aggregator->metrics_file, "%ld", nr) == 0) {
    perror("Error on reading metrics file");
    exit(-1);
  }

  fclose(aggregator->metrics_file);

  aggregator->old_nr_of_metrics = aggregator->total_metrics;

  aggregator->total_metrics = *nr;

  if(aggregator->desired_metrics) {

    for(int i = 0; i < aggregator->old_nr_of_metrics; ++i) {
        free(aggregator->desired_metrics[i]);
        aggregator->desired_metrics[i] = NULL;
    }

    free(aggregator->desired_metrics);

    aggregator->desired_metrics = NULL;
  }

  aggregator->desired_metrics = (char **) calloc (aggregator->total_metrics, sizeof(char *));
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void initialize_metrics_crawler_number_from_memory(metrics_aggregator_t *aggregator, long *nr)
{
  *nr = aggregator->total_metrics;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void metrics_crawler_results_memory(metrics_aggregator_t *aggregator, aggregators_t *result)
{
  compute_metric(aggregator->desired_metrics, result, aggregator->total_metrics, 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int metrics_crawler_results_file(metrics_aggregator_t *aggregator, aggregators_t *result, char *filename)
{
  aggregator->metrics_file = fopen(filename, "r");
  if(!aggregator->metrics_file) {
    perror("Error on opening metric aliases file");
    exit(-1);
  }

  char line[MAX_LENGTH_ALIAS];

  long metric_number = 0;

  long number = 0;

  int needs_sync = false;

  if (aggregator->metrics_file == NULL) {
    exit(-1);
  }

  if(fscanf(aggregator->metrics_file, "%ld\n", &number) == -1) {
    fprintf(stderr, "%s\n", "Error while reading metrics file");
    exit(-1);
  }

  while (fscanf(aggregator->metrics_file, "%s\n", line) != -1 &&
         metric_number < aggregator->total_metrics) {
    if((number != aggregator->old_nr_of_metrics) ||
       (aggregator->desired_metrics[metric_number] &&
        strcmp(aggregator->desired_metrics[metric_number], line))) {
      needs_sync = true;
    }

    aggregator->desired_metrics[metric_number] = strdup(line);

    ++metric_number;
  }

  compute_metric(aggregator->desired_metrics, result, aggregator->total_metrics, 1);

  fclose(aggregator->metrics_file);

  return needs_sync;
}

////////////////////////////////////////////////////////////////////////////////////////////////////