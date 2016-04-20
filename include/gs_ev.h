/*******************************************************************************
* File:     gs_ev.h
* Purpose:  Specify the interface for interactions with GroupServer.  GS is an
*           interface to a Proactive Directory Service (PDS) daemon.  This 
*           interface assumes that GS is being used to register names based on
*           the kind of information the name represents rather than by the
*           application publishing it.  For example, the interface would
*           register "videostreamTypeA" rather than "videostreamingAppXYZ".
*
* Notes:    PDS_SERVER_HOST and PDS_SERVER_PORT environment variables may be set
*           to specify the PDS daemon of choice.  The defaults for these may be
*           changed in the configure script.  Use "configure --help" for more
*           details.
*
* Warnings: Many/Most of the functions returned malloc'd strings of the entry
*           which was made in GS.  The caller must free them!
*
*******************************************************************************/

#ifndef _GS_EV_H_
#define _GS_EV_H_

/*******************************************************************************
* INCLUDES
*******************************************************************************/
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <pwd.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <unistd.h>

#include "ffs.h"
#include "echo2.h"
#ifdef HAVE_CONFIG_H
  #include "config.h"
#endif


/*******************************************************************************
* DEFINES
*******************************************************************************/

/* Context Strings */
#define GS_CONTEXT_ROOT            "/"
#define GS_CONTEXT_CHANNELS        "channels"
#define GS_CONTEXT_NODES           "nodes"
#define GS_CONTEXT_SINKS           "sinks"
#define GS_CONTEXT_SOURCES         "sources"
#define GS_CONTEXT_WAITING         "waiting"

/* Entity Strings */
#define GS_ENTITY_HEARTBEAT        "gs_heartbeat"
#define GS_ENTITY_APP_NAME         "gs_app_name"
#define GS_ENTITY_HOST             "gs_host"
#define GS_ENTITY_CREATE_TIME      "gs_create_time"
#define GS_ENTITY_OWNER            "gs_owner"
#define GS_ENTITY_PID              "gs_pid"
#define GS_ENTITY_FILTER           "gs_filter"

/* EVPath Strings */
#define GS_ENTITY_EV_CONTACT_LIST  "gs_ev_cl"
#define GS_ENTITY_EV_TYPE          "gs_ev_type"

/* Xpath Query Operators */
#define GS_XPATH_AND_JOIN          0
#define GS_XPATH_OR_JOIN           1

/* Return Values */
#define SUCCESS                    1
#define FAILURE                    0

/*******************************************************************************
* FUNCTION PROTOTYPES
*******************************************************************************/

/****************************
* Initialization & Shutdown *
****************************/

/* gs_init() - Initialize a connection to the PDS daemon using the environment
 *    variables PDS_SERVER_HOST and PDS_SERVER_PORT.
 *    RETURNS: SUCCESS or FAILURE.
 */
int gs_init(void);

/* gs_shutdown() - Terminates the PDS connection resulting from gs_init().  The
 *    gs_init() function must be called again in order to use the groupserver
 *    interface functions.
 */
void gs_shutdown(void);


/**********************************************
* Context & Entity Addition/Removal - Generic *
**********************************************/

/* gs_context_add() - Adds a new_context at base_path. base_path must start "/".
 *    RETURNS: the full path or NULL on an error.
 *    NOTES:   the user must free the returned string.
 */
char *gs_context_add(char *base_path, char *new_context);

/* gs_context_remove() - Removes the trailing component of target_path from PDS.
 *    RETURNS: SUCCESS or FAILURE; FAILURE indicates a communications problem
 *    with PDS.  Note: remove of a non-existing target will succeed!
 */
int gs_context_remove(char *target_path);

/* gs_entity_add() - Adds an entity with value at base_path/relative_context.
 *    A relative_context of NULL or "" implies entity creation at base_path.
 *    RETURNS: the full path to the entity or NULL on error.
 *    NOTES:   the user must free the returned string.
 */
char *gs_entity_add(char *base_path, char *relative_context, char *entity, char *value);

/* gs_entity_remove() - Removes the trailing component of target_path from PDS.
 *    RETURNS: SUCCESS or FAILURE (unknown entity or communications problem).
 */
int gs_entity_remove(char *target_path);

/* gs_entity_set_value() - Sets the value of the entity at entity_path.
 *    RETURNS: SUCCESS or FAILURE; (unknown entity or communications problem).
 */
int gs_entity_set_value(char *entity_path, char *value);

/* gs_entity_get_value() - Gets the value of the entity at entity_path.
 *    RETURNS: the value or NULL (unknown entity or communications problem).
 *    NOTE: the user must free the returned string.
 */
char *gs_entity_get_value(char *entity_path);

/* gs_register_for_context_changes() - Registers a handler function to be used
 *    as a callback when changes occur on the context specified by context_path.
 *    The handler will be called with the value of the client_data pointer as 
 *    it's argument.  If the same handler were registered for several contexts
 *    the client_data could be used to indicate the context change on which the
 *    handler is being called.
 *    RETURNS: SUCCESS or FAILURE (unknown context or communications problem).
 */
int gs_register_for_context_changes(CManager listenerCM, char *context_path,
                                    ECTypedHandlerFunction handler, void *client_data);


/*********************************************************
* Context & Entity Addition/Removal - EVPath specialized *
*********************************************************/

/*
 * gs_add_ev_type() - Adds the context "/GS_CONTEXT_CHANNEL/ev_type" to PDS.
 *    Additionally, contexts GS_CONTEXT_SOURCE and GS_CONTEXT_SINK are added at
 *    the ev_type context.  An entity of GS_ENTITY_EV_TYPE with value ev_type is
 *    created at this location as well.
 *    RETURNS: ev_type's full path or NULL on error (partial or total failure).
 *    NOTE: the user must free the returned string.
 */
char *gs_add_ev_type(char *ev_type);

/*
 * gs_add_ev_source() - Adds the calling process as a supplier of data of type
 *    ev_type.  The ev_type argument must match a type previously added by
 *    gs_add_ev_type().  The contact_list argument will be used for sinks to
 *    contact the source.  If NULL, a sink waiting for a source to appear will
 *    be unable to contact this source (see gs_receive_new_ev_sources).  The 
 *    contact_list must be formatted  "inputstone:CMcontactstring".
 *    RETURNS: the full path that was added or NULL on an error.
 *    NOTE: the user must free the returned string.
 */
char *gs_add_ev_source(char *ev_type, char *contact_list);

/*
 * gs_add_ev_sink() - Adds the calling application as a sink for data of type
 *    ev_type.  The ev_type argument must match a type previously added by
 *    gs_add_ev_type().   The contact_list format "inputstone:CMcontactstring"
 *    is expected; NULL is disallowed.
 *    RETURNS: the full path that was added or NULL on error.
 *    NOTE: the user must free the returned string.
 */
char *gs_add_ev_sink(char *ev_type, char *contact_list);

/*
 * gs_add_ev_sink() - Adds the calling application as a sink for data of type
 *    ev_type.  The ev_type argument must match a type previously added by
 *    gs_add_ev_type().   The contact_list format "inputstone:CMcontactstring"
 *    is expected; NULL is disallowed.
 *    RETURNS: the full path that was added or NULL on error.
 *    NOTE: the user must free the returned string.
 */
char *gs_add_ev_sink_filtered(char *ev_type, char *contact_list, char *filter);

/*
 * gs_ev_source_adding_sink() - Takes care of cross listing the sink as a
 *    consumer of the source and the source as a provider of the sink.  The
 *    second argument of this function is the string that is returned from 
 *    gs_get_app_info() when called from the source process.
 *    RETURNS: SUCCESS or FAILURE.
 *    NOTE: argument format will be "/channels/evtype/sinks/specific_app_string"
 */ 
int gs_ev_source_adding_sink(char *full_sink_path, char *src_app_info);

/*
 * gs_receive_new_ev_sources() - This is a convenience function which closely
 *    mimics gs_register_for_context_changes().  The difference here is that 
 *    the ev_type argument is expanded to a full context path internally.  This
 *    path is of the form "full_ev_type_path/GS_CONTEXT_SOURCES".
 *    RETURNS: SUCCESS or FAILURE (unknown context or communications problem).
 */
int gs_receive_new_ev_sources(CManager listenerCM, char *ev_type,
                              ECTypedHandlerFunction handler, void *client_data);


/*
 * gs_receive_new_ev_sinks() - This is a convenience function which closely
 *    mimics gs_register_for_context_changes().  The difference here is that
 *    the ev_type argument is expanded to a full context path internally.  This
 *    path is of the form "full_ev_type_path/GS_CONTEXT_SINKS".
 *    RETURNS: SUCCESS or FAILURE (unknown context or communications problem).
 */
int gs_receive_new_ev_sinks(CManager listenerCM, char *ev_type,
                            ECTypedHandlerFunction handler, void *client_data);


/*
 * gs_ev_add_waiting_sink() - Creates "/channels/<evtype>/sinks/waiting/<appinfo>"
 *    The intention of the function is to be able to list the sinks which are waiting
 *    for a corresponding source.
 * NOTE: user must free the returned string.
 */
char *gs_ev_add_waiting_sink(char *ev_type, char *complete_contact_info);

/*
 * gs_ev_get_waiting_sink_contact() - returns the complete_contact_info value that
 *    was stored with the waiting sink.
 * NOTE: user must free the returned string.
 */
char *gs_ev_get_waiting_sink_contact(char *ev_type, char *app_info);

/*
 * gs_ev_remove_waiting_sink() - Removes the sink from the waiting listing.
 */
int gs_ev_remove_waiting_sink(char *ev_type, char *app_info);


/**************************
* XPath Queries - Generic *
**************************/

/* gs_get_latest_context() - Returns a pointer to the most recently created
 *                           context from the passed list of contexts.  Returns
 *                           NULL on an error.
 * WARNING: there is no source for this!  Similar functionality exists via
 *          gs_get_latest_ev_source() --> see the backing .c file if you need to
 *          get this functionality working.
 */
char *gs_get_latest_context(char** context_list);


/* gs_build_xpath_query() - Returns an XPath query based on the specified join
 *    operation where the query string will be the OR/AND of the ev_type with
 *    the criteria.
 *    WARNING: look at the source yourself!
 *    NOTE: user must free the returned string.
 */
char *gs_build_xpath_query(char *ev_type, pds_query_struct* criteria, int join_operation);

/* gs_build_extended_xpath_query() - Returns an XPath query string.
 *    WARNING: look at the source yourself!
 *
 */
char *gs_build_extended_xpath_query(char *ev_type, pds_query_struct* criteria[], int num);


/* gs_query_unique_key_list() - run a query created with one of the XPath query
 *    building functions.
 */
char **gs_query_unique_key_list(char* xpath_query);


char **gs_get_unique_key_list_blocking(char* app_name, char* uid[], long timeout);

/* gs_get_fullpath_entity_value() - Gets the value of the specified entity at
 *    the path provided.  It returns a string, or NULL if the entity does not
 *    exist or that entity's value is NULL (zero length).
 * NOTE: the user must free the returned string.
 */
char *gs_get_fullpath_entity_value(char *path, char *entity);

/* gs_get_entity_value_blocking() - Gets the value of the specified entity at
 *    the path provided.  It provides the value once it becomes available.  It
 *    will continue to poll for a value until either one is found or the timeout
 *    time has expired. A timeout of 0 indicates wait forever.
 * NOTE: the user must free the returned string.
 */
char *gs_get_entity_value_blocking(char *base_context, char *relative_context, char *entity, long timeout);


/*************************************
* XPath Queries - EVPath specialized *
*************************************/

char *gs_ev_get_my_latest_source(char* type); 

char *gs_ev_get_latest_source(char* type); 


/*****************
* Heartbeat APIs *
*****************/

/* gs_heartbeat_pulse() - Sends a heartbeat to the pds daemon to keep the
 *      specified context alive in the pds registry.  The first call on a 
 *      particular context will create the heartbeat entity for the key's 
 *      context. Contexts become eligible for garbage collection once an 
 *      initial heartbeat has been sent.
 *      RETURNS: SUCCESS or FAILURE; FAILURE is likely due to a communications
 *      problem.
 *      NOTE: PDS daemon garbage collects non-heartbeated contexts after 10 mins.
 */
int gs_heartbeat_pulse(char *target_context);

/* gs_get_heartbeat() - Return the time of the most recent heartbeat on the
 *      given context. The time is returned as a string representation of the
 *      seconds since the UNIX epoch.  Returns NULL if the context has no
 *      associated heartbeat.
 * NOTE: the user must free the returned string.
 */
char *gs_get_heartbeat(char *target_context);


/*****************
* MISCELLANEOUS  *
*****************/

/* gs_get_app_info() - Returns the string associated with this process in gs.
 * NOTE: the user must free the returned string.
 */
char *gs_get_app_info(void);

char **gs_get_all_ev_source(char *ev_type, char *uid);
char ** gs_get_all_ev_sinks(char *ev_type, char *uid) ;
/*******************************************************************************
* END OF HEADER
*******************************************************************************/
#endif
