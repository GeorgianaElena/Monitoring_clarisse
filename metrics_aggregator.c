#include "metrics_aggregator.h"

int main(int argc, char **argv)
{
  int rank, nprocs;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

  char* parents_addr[nprocs];
  initialize_monitoring(parents_addr);
  
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

CManager compute_evpath_addr(char *addr)
{
  CManager cm;
  EVstone stone;
  char *string_list_addr;
  cm = CManager_create();
  CMlisten(cm);

  stone = EValloc_stone(cm);
  EVassoc_terminal_action(cm, stone, simple_format_list, simple_handler, NULL);
  string_list_addr = attr_list_to_string(CMget_contact_list(cm));
  
  sprintf(addr, "%d:%s", stone, string_list_addr);

  return cm;
}

void initialize_monitoring(char* parents_addr[])
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
    CManager cm = compute_evpath_addr(myaddr);

    send_addr_to_parent(myaddr);

    CMrun_network(cm); 
  }
}