#ifndef __LGSDATASTRUCTS_H__
#define __LGSDATASTRUCTS_H__


#include "lgs.h"
#include <pthread.h>
#define thr_mutex_t pthread_mutex_t
#define thr_thread_t pthread_t
#define thr_condition_t pthread_cond_t
#define thr_thread_self() pthread_self()
#define thr_thread_exit(status) pthread_exit(status);
#define thr_thread_detach(thread) pthread_detach(thread);
#define thr_thread_yield() pthread_yield();
#define thr_thread_join(t, s) pthread_join(t, s)
#define thr_mutex_init(m) pthread_mutex_init(&m, NULL);
#define thr_mutex_lock(m) pthread_mutex_lock(&m);
#define thr_mutex_unlock(m) pthread_mutex_unlock(&m);
#define thr_mutex_free(m) pthread_mutex_destroy(&m);
#define thr_condition_init(c) pthread_cond_init(&c, NULL);
#define thr_condition_wait(c, m) pthread_cond_wait(&c, &m);
#define thr_condition_signal(c) pthread_cond_signal(&c);
#define thr_condition_broadcast(c) pthread_cond_broadcast(&c);
#define thr_condition_free(c) pthread_cond_destroy(&c);

#define PUBLISH_NODE 1
#define OUTPUT_NODE 2

typedef struct list_item
{
	void *item;
	struct list_item * next;
} list_item;

typedef int (*Comparator_Func)(void *a, void *b);

typedef struct submit_handle
{
	EVsource source;
	EVstone pub_stone;
	EVaction pub_action;
} submit_handle;

typedef struct output_item
{
	char sub_appname[MAX_APPLN_NAME];
	EVstone output_stone;
} output_item;

typedef struct publisher_item
{
  submit_handle * handle;
  EVstone filter_stone;
  EVaction filter_action;
  EVstone split_stone;
  EVaction split_action;
} publisher_item;

typedef struct subscriber_item
{
	char * channel_name;
	EVstone sub_stone;
	char * contact_list;
} subscriber_item;

typedef struct lgs_tree_
{
  char * name;
  int type;
  void * item;
  struct lgs_tree_ * child;
  struct lgs_tree_ * sibling;
} lgs_tree;

void setAppName(char * app);
void * list_view(void * item, list_item * head, thr_mutex_t * mutex, Comparator_Func compare);
void * list_retrieve(void *item, list_item **head, thr_mutex_t *mutex, Comparator_Func compare);
void list_insert(void * item, list_item ** list_head, thr_mutex_t * list_mutex);
int publisher_name_compare(void * compare_name, void * pub_item);

void initTree(lgs_tree ** head, thr_mutex_t * mutex);
lgs_tree * traversePath(lgs_tree * node, char * path, thr_mutex_t * mutex);
int addNode(lgs_tree * head, publisher_item * pub_item, char * path, char * name, thr_mutex_t * mutex);
int addLeaf(lgs_tree * head, output_item * out_item, char * path, thr_mutex_t * mutex);
int insertNode(lgs_tree * parent_item, publisher_item * fitem, char * filter_name, char * sub_appname, CManager cm, thr_mutex_t * mutex);
int findSubscriberInTree(lgs_tree * node, char * name);
int getSubscribersInTree(lgs_tree * node, char **subscribers, int * n);

#endif
