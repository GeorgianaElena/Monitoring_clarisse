CC = mpicc
CFLAGS = -std=gnu11 -O3 -Wall -g

build: metrics_aggregator

metrics_aggregator: metrics_aggregator.c metrics_aggregator.h available_metrics.h storage.h metrics_crawler.h
	$(CC) $(CFLAGS) metrics_aggregator.c -o metrics_aggregator -lpapi

clean:
	rm -f metrics_aggregator
