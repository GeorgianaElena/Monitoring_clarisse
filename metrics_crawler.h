#ifndef METRICS_CRAWLER_H
#define METRICS_CRAWLER_H

#include "available_metrics.h"
#include "storage.h"
#include "stdio.h"

static FILE *metrics_file;

typedef struct _aggregators_t aggregators_t;

void initialize_metrics_crawler()
{
	init();
}

void initialize_metrics_crawler_number(long *nr)
{
    metrics_file = fopen ("file.txt", "r");
  	
  	fscanf (metrics_file, "%ld", nr);

    fclose(metrics_file);  	  
}

void initialize_metrics_crawler_results(aggregators_t *result)
{
    metrics_file = fopen("file.txt", "r");
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

	if (metrics_file == NULL) {
	   exit(EXIT_FAILURE);
	}

	long metric_number = 0;

	/* skip first line */
	getline(&line, &len, metrics_file);

	while ((read = getline(&line, &len, metrics_file)) != -1) {
		double (*func)();
	    func = get_value(line);

	    long metric_result = func();

	    result[metric_number].min = metric_result;
	    result[metric_number].max = metric_result;
	    result[metric_number].sum = metric_result;

	    ++metric_result;
	}

	fclose(metrics_file);
}

#endif