#ifndef __LGSCOMMLIB_H__
#define __LGSCOMMLIB_H__

#include "lgsdatastructs.h"
#include <evpath.h>
#include <cod.h>

typedef struct properties_
{
	EVstone stone;
	CManager cm;
	EVaction action;
}Properties;


extern CManager init(char * appname, int listen_port);
extern CManager LGS_getCM();
extern submit_handle * create_publisher(char * pub_name, FMStructDescRec * format);
extern int create_subscriber(char * pub_name, FMStructDescList format, EVSimpleHandlerFunc handler);
extern int create_filtered_subscriber(char *pub_name, FMStructDescList pub_format, char * filter_name, char * filter, FMStructDescList filtered_format, EVSimpleHandlerFunc handler);
extern int delete_publisher(char * path, char * sub_appname);
extern submit_handle * create_subscriber_mq(char * pub_name, FMStructDescRec* format, char* subtype, char* mq_function, char* out_subtype);
extern int publish_msg(submit_handle * handle, void * msg);
extern void get_buffer(void * msg);
extern void return_buffer(void * msg);

extern int checkForSubscriber(char * sub_appname);
extern int createListOfSubscribers(char ** subscribers);

//struct timeval end_timer;

#endif
