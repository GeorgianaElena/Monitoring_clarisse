CC = mpicc
CFLAGS = -Wall -g -I $$HOME/clarisse/build_area/evpath/include
LDFLAGS = -levpath -L $$HOME/clarisse/lib -latl -lffs -lcercs_env -ldill

build: metrics_aggregator

metrics_aggregator: metrics_aggregator.c
	$(CC) $(CFLAGS) metrics_aggregator.c -o metrics_aggregator $(LDFLAGS)

clean:
	rm -f metrics_aggregator
