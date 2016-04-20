CC = mpicc
CFLAGS = -O3 -Wall -g -I ./include
LDFLAGS = -levpath -L ./lib -latl -lffs -lcercs_env -ldill

build: metrics_aggregator

metrics_aggregator: metrics_aggregator.c metrics_aggregator.h available_metrics.h storage.h metrics_crawler.h
	$(CC) $(CFLAGS) metrics_aggregator.c -o metrics_aggregator $(LDFLAGS)

clean:
	rm -f metrics_aggregator
