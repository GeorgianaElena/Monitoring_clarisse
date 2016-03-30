#include "metrics_aggregator.h"
#include "metrics_crawler.h"

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
  printf("Min = %f\tMax = %f\tAverage = %f\n", event->gather_info[0].min,
          event->gather_info[0].max, event->gather_info[0].sum);
  return 0;
}

static int compute_own_metrics(CManager cm, void *vevent, void *client_data, attr_list attrs)
{

  int rank;

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  static int count = 0;
  ++count;
  if(count == DEGREE) {
    EVsource source = EVcreate_submit_handle(current_state.conn_mgr, current_state.multi_stone,
                                             metrics_format_list);
    
    initialize_metrics_crawler();

    metrics_t data;
    data.degree = DEGREE;
    
    initialize_metrics_crawler_number(&data.metrics_nr);

    data.gather_info = malloc(data.metrics_nr * sizeof(aggregators_t));
    initialize_metrics_crawler_results(data.gather_info);

    EVsubmit(source, &data, NULL);
   
    count = 0;
  }

  return 0;
}

static int is_leaf()
{
  int rank;
  int nprocs;

  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  return ((rank * DEGREE + 1 >= nprocs) && (nprocs > 1)); 
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

void send_addr_to_children(char *addr)
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
  current_state.split_stone = EValloc_stone(current_state.conn_mgr);

  string_list_addr = attr_list_to_string(CMget_contact_list(current_state.conn_mgr));
  
  sprintf(addr, "%d:%s", current_state.split_stone, string_list_addr);

  memcpy(current_state.own_addr, addr, ADDRESS_SIZE);
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

    send_addr_to_children(myaddr);
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
    current_state.agreg_terminal_stone = EValloc_stone(current_state.conn_mgr);
    EVassoc_terminal_action(current_state.conn_mgr, current_state.agreg_terminal_stone,
                            metrics_format_list, final_result, NULL);
    EVstone_set_output(current_state.conn_mgr, current_state.multi_stone, 0,
                       current_state.agreg_terminal_stone);
  }

  /* create the multistone */
  if(!is_leaf()) {
      CManager cm = current_state.conn_mgr;
      EVstone terminal_stone;

      current_state.terminal_stone = EValloc_stone(cm);
      terminal_stone = current_state.terminal_stone;
      
      EVassoc_terminal_action(current_state.conn_mgr, terminal_stone,
                            metrics_format_list, compute_own_metrics, NULL);

      EVaction split_action = EVassoc_split_action(cm, current_state.split_stone, NULL);
      
      EVaction_add_split_target(cm, current_state.split_stone, split_action, terminal_stone);
      EVaction_add_split_target(cm, current_state.split_stone, split_action,
                                current_state.multi_stone);

    static char *multi_func = "{\n\
      int found = 0;\n\
      metrics_t *a;\n\
      int degree = -1;\n\
      if (EVcount_metrics_t()) {\n\
          a = EVdata_metrics_t(0);\n\
          degree = a->degree;\n\
      }\n\
      \n\
      if (EVcount_metrics_t() == degree + 1) {\n\
          int i;\n\
          \n\
          metrics_t c;\n\
          c.metrics_nr = a->metrics_nr;\n\
          c.gather_info[0].min = a->gather_info[0].min;\n\
          c.gather_info[0].max = a->gather_info[0].max;\n\
          c.gather_info[0].sum = a->gather_info[0].sum;\n\
          c.degree = a->degree;\n\
          \n\
          while(EVcount_full()) {\n\
            EVdiscard_metrics_t(0);\n\
            if(EVcount_metrics_t() > 0) {\n\
              a = EVdata_metrics_t(0);\n\
              c.gather_info[0].sum += a->gather_info[0].sum;\n\
              if(c.gather_info[0].min > a->gather_info[0].min) {\n\
                c.gather_info[0].min = a->gather_info[0].min;\n\
              }\n\
              \n\
              if(c.gather_info[0].max < a->gather_info[0].max) {\n\
                c.gather_info[0].max = a->gather_info[0].max;\n\
              }\n\
            }\n\
          }\n\
          /* submit the new, combined event */\n\
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

  EVsource source = EVcreate_submit_handle(current_state.conn_mgr, current_state.bridge_stone,
                                           metrics_format_list);
  
  /* populate the storage data stucture with the metrics known by the program so far */
  initialize_metrics_crawler();
  
  metrics_t data;
  data.degree = DEGREE;
  
  initialize_metrics_crawler_number(&data.metrics_nr);

  data.gather_info = malloc(data.metrics_nr * sizeof(aggregators_t));
  initialize_metrics_crawler_results(data.gather_info);

  EVsubmit(source, &data, NULL);

  /* send data periodically*/
  while (1) {
    sleep(1);

    initialize_metrics_crawler_number(&data.metrics_nr);

    data.gather_info = malloc(data.metrics_nr * sizeof(aggregators_t));
    initialize_metrics_crawler_results(data.gather_info);

    EVsubmit(source, &data, NULL);
  }
}