CC = mpicc
CFLAGS = -std=gnu11 -O3 -Wall -g -I ./include
LDFLAGS = -levpath -L ./lib -latl -lffs -lcercs_env -ldill -L /usr/local/lib -lpapi

MONITORING_LIB = libevp_monitoring.so

SRCS    = metrics_aggregator.c evp_monitoring.c helpers.c metrics_crawler.c metric_type.c storage.c
HEADERS = metrics_aggregator.h evp_monitoring.h helpers.h metrics_crawler.h metric_type.h storage.h uthash.h
OBJS    = metrics_aggregator.o evp_monitoring.o helpers.o metrics_crawler.o metric_type.o storage.o
TEST    = test.c

all: $(MONITORING_LIB)

$(MONITORING_LIB): $(OBJS)
	$(CC) $(CFLAGS) $^ -shared -o $@ $(LDFLAGS)

metrics_aggregator.o: metrics_aggregator.c
	$(CC) $(CFLAGS) -c -fPIC metrics_aggregator.c

evp_monitoring.o: evp_monitoring.c
	$(CC) $(CFLAGS) -c -fPIC evp_monitoring.c

helpers.o: helpers.c
	$(CC) $(CFLAGS) -c -fPIC helpers.c

metrics_crawler.o: metrics_crawler.c
	$(CC) $(CFLAGS) -c -fPIC metrics_crawler.c

metric_type.o: metric_type.c
	$(CC) $(CFLAGS) -c -fPIC metric_type.c

storage.o: storage.c
	$(CC) $(CFLAGS) -c -fPIC storage.c

test: $(MONITORING_LIB)
	$(CC) $(CFLAGS) $(TEST) -o $@ $(LDFLAGS) -L. -levp_monitoring

# Running example
run: test
	mpiexec -n 10 ./test ./input_files/file.txt 5 2 500000

clean: 
	rm -f $(MONITORING_LIB) $(OBJS) test