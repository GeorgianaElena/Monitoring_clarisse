#include "metrics_aggregator.h"

static node_state_t current_state;

int main(int argc, char **argv)
{
  int rank, nprocs;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

  initialize_monitoring();
  MPI_Barrier(MPI_COMM_WORLD);

  create_stones();

  start_communication();

  MPI_Finalize();

  return 0; 
}

static int final_result(CManager cm, void *vevent, void *client_data, attr_list attrs)
{
  metrics_t_ptr event = vevent;
  printf("I got %d\n", event->integer_field);

  return 0;
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

void compute_evpath_addr(char *addr)
{
  int rank;

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  char *string_list_addr;
  current_state.conn_mgr = CManager_create();
  CMlisten(current_state.conn_mgr);

  current_state.multi_stone = EValloc_stone(current_state.conn_mgr);
  string_list_addr = attr_list_to_string(CMget_contact_list(current_state.conn_mgr));
  
  sprintf(addr, "%d:%s", current_state.multi_stone, string_list_addr);
}

void initialize_monitoring()
{
  int rank;

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if(rank != 0) {
    /* Receive evpath address from parent node */
    char addr[ADDRESS_SIZE];
    recv_addr_from_parent(addr);

    memcpy(current_state.parent_addr, addr, ADDRESS_SIZE);
  }

  if(!is_leaf()) {
    /* Compute evpath address and send to children */
    char myaddr[ADDRESS_SIZE];
    compute_evpath_addr(myaddr);

    send_addr_to_parent(myaddr);
  }
}

void create_stones()
{
  int rank;

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if(rank != 0) {
    //create the bridge stone

    if(is_leaf()) {
      current_state.conn_mgr = CManager_create();
      CMlisten(current_state.conn_mgr);
    }

    CManager cm = current_state.conn_mgr;

    current_state.bridge_stone = EValloc_stone(cm);
    EVstone output = current_state.bridge_stone;

    char string_list[ADDRESS_SIZE];
    EVstone remote_stone;
    
    if (sscanf(current_state.parent_addr, "%d:%s", &remote_stone, &string_list[0]) != 2) {
      printf("Bad argument \"%s\"\n", current_state.parent_addr);
      exit(0);
    }

    attr_list contact_list = attr_list_from_string(string_list);

    EVassoc_bridge_action(cm, output, contact_list, remote_stone);
    EVstone_set_output(cm, current_state.multi_stone, 0, output);
  } else {
    current_state.terminal_stone = EValloc_stone(current_state.conn_mgr);

    EVassoc_terminal_action(current_state.conn_mgr, current_state.terminal_stone,
                            metrics_format_list, final_result, NULL);
    EVstone_set_output(current_state.conn_mgr, current_state.multi_stone, 0, current_state.terminal_stone);
  }

  //create the multistone

  if(!is_leaf()) {
    static char *multi_func = "{\n\
      int found = 0;\n\
      metrics_t *a;\n\
      int degree = -1;\n\
      if (EVcount_metrics_t()) {\n\
          a = EVdata_metrics_t(0);\n\
          degree = a->degree;\n\
      }\n\
      \n\
      if (EVcount_metrics_t() == degree) {\n\
          int i;\n\
          metrics_t c;\n\
          for(i = 0; i < degree; i++) {\n\
            a = EVdata_metrics_t(i);\n\
            c.integer_field += a->integer_field;\n\
            /* submit the new, combined event */\n\
          }\n\
          for(i = 0; i < degree; i++) {\n\
            /* discard the used events */\n\
            EVdiscard_metrics_t(0);\n\
          }\n\
          \n\
          c.degree = degree;\n\
          EVsubmit(0, c);\n\
      }\n\
    }\0\0";  

    char *mq = create_multityped_action_spec(queue_list, multi_func);
    EVassoc_multi_action(current_state.conn_mgr, current_state.multi_stone, mq, NULL);
    
    CMrun_network(current_state.conn_mgr);
  }
}

void start_communication()
{
  int rank;

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  EVsource source = EVcreate_submit_handle(current_state.conn_mgr, current_state.bridge_stone, metrics_format_list);
  metrics_t data;
  data.integer_field = rank;
  data.degree = DEGREE;

  while (1) {
    EVsubmit(source, &data, NULL);
    sleep(2);
  }
}