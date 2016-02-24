#include "metrics_aggregator.h"

int main(int argc, char **argv)
{
  int rank, nprocs;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

  char* parents_addr[nprocs];
  CManager connection_managers[nprocs];

  initialize_monitoring(parents_addr, connection_managers);
  send_to_parent(parents_addr, connection_managers);
  
  MPI_Finalize();

  return 0; 
}

static int simple_handler(CManager cm, void *vevent, void *client_data, attr_list attrs)
{
  simple_rec_ptr event = vevent;
  printf("I got %d\n", event->integer_field);

  return 1;
}

static int is_leaf()
{
  int rank;
  int nprocs;

  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  return (rank * DEGREE + 1 >= nprocs); 
}

static int get_parent()
{
  int rank;
  int nprocs;

  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  return (rank - 1) / DEGREE;
}

void recv_addr_from_parent(char *addr)
{
  int rank;

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  MPI_Recv(addr, ADDRESS_SIZE, MPI_CHAR, get_parent(), 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  printf("Process %d received from parent %s\n", rank, addr);
}

void send_addr_to_parent(char *addr)
{
  int rank, tag = 0;

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  for(int i = 1; i <= DEGREE; ++i) {
    MPI_Send(addr, ADDRESS_SIZE, MPI_CHAR, rank * DEGREE + i, tag, MPI_COMM_WORLD);
  }
}

void compute_evpath_addr(char *addr, CManager* connection_managers)
{
  int rank;

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  CManager cm;
  EVstone stone;
  char *string_list_addr;
  cm = CManager_create();
  CMlisten(cm);

  stone = EValloc_stone(cm);
  EVassoc_terminal_action(cm, stone, simple_format_list, simple_handler, NULL);
  string_list_addr = attr_list_to_string(CMget_contact_list(cm));
  
  sprintf(addr, "%d:%s", stone, string_list_addr);

  connection_managers[rank] = cm;
}

void initialize_monitoring(char* parents_addr[], CManager* connection_managers)
{
  int rank;

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if(rank != 0) {
    /* Receive evpath address from parent node */
    char addr[ADDRESS_SIZE];
    recv_addr_from_parent(addr);

    parents_addr[rank] = addr;
  }

  if(!is_leaf()) {
    /* Compute evpath address and send to children */
    char myaddr[ADDRESS_SIZE];
    compute_evpath_addr(myaddr, connection_managers);

    send_addr_to_parent(myaddr);

    if(rank == 0) {
      CMrun_network(connection_managers[rank]);
    }
  }
}

void send_to_parent(char* parents_addr[], CManager* connection_managers)
{
  int rank;

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if(rank != 0) {
    /* Send rank evpath */
    CManager cm;
    simple_rec data;
    EVstone stone;
    EVsource source;
    char string_list[2048];
    attr_list contact_list;
    EVstone remote_stone;

    if (sscanf(parents_addr[rank], "%d:%s", &remote_stone, &string_list[0]) != 2) {
        printf("Bad arguments \"%s\"\n", parents_addr[rank]);
        exit(0);
    }

    if(is_leaf()) {
      cm = CManager_create();
      CMlisten(cm);
      stone = EValloc_stone(cm);
    } else {
      cm = connection_managers[rank];
      CMlisten(cm);
      stone = EValloc_stone(cm);
    }    

    contact_list = attr_list_from_string(string_list);
    EVassoc_bridge_action(cm, stone, contact_list, remote_stone);

    source = EVcreate_submit_handle(cm, stone, simple_format_list);
    data.integer_field = rank;
    EVsubmit(source, &data, NULL);

    CMrun_network(cm);
  }
}