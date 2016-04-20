/* -*-C-*-
*******************************************************************************
* 
* File:        comm_group2.h
* RCS:         $Id: comm_group2.h,v 1.1.1.1 2000-04-30 21:09:48 fabianb Exp $
* Description: New cm-based comm_group I/F, derived from greg's code.
* Author:      Fabian E. Bustamante (fabianb@cc.gatech.edu)
*              Systems Research Group
*              College of Computing
*              Georgia Institute of Technology
* Created:
* Modified:
*  
* (C) Copyright 2000, all rights reserved.
*
*******************************************************************************
*/
#ifndef __COMM_GROUP__H__
#define __COMM_GROUP__H__

#if defined(FUNCPROTO) || defined(__STDC__) || defined(__cplusplus) || defined(c_plusplus)
#ifndef ARGS
#define ARGS(args) args
#endif
#else
#ifndef ARGS
#define ARGS(args) (/*args*/)
#endif
#endif

typedef struct _comm_group_struct {
  char *user_name;
  char *application_name;
  char *group_type;
  char *group_id;
  char *host;
  int port;
  int info_len;
  void *info;
} comm_group_struct, *comm_group_list;

typedef struct _comm_group_return {
  int cond;			/* no ideal but could be worst */
  int count;
  comm_group_list list;
} ret_group_struct, *comm_group_return;


extern char * 
setup_comm_group ARGS((char *user_name, char *application_name, char *group_type, 
		       int expiration_time, char *host, int port));

extern char * 
setup_specific_comm_group ARGS((char *application_name, char *group_type, 
				int expiration_time, char *host, int port));

extern int 
init_comm_group_contact ARGS((char *user_name, char *application_name,
			      char *group_type, char **host, int *port));

extern int 
init_specific_comm_group_contact ARGS((char *group_id, char **host, int *port));

extern int 
group_server_present();

extern int 
group_server_test();

extern void 
assoc_group_info ARGS((char *group_name, void *data, int data_len));

extern comm_group_return 
matching_comm_groups ARGS((char *app_list, char *type_list));

extern CMConnection 
CMConnection_initiate_first ARGS((CManager cm, comm_group_return groups));

typedef void (*group_error_routine_type) ();

extern void 
set_comm_group_error_handler ARGS((group_error_routine_type error_routine));

extern void 
comm_group_set_app_name ARGS((char *app_name));

extern void 
comm_group_set_app_list ARGS((char *list));

extern 
void comm_group_set_comm_type ARGS((char *comm_type));

extern 
void comm_group_set_type_list ARGS((char *list));

#endif
