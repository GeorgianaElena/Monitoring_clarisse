/* $Id: echo2.h,v 1.2 2008-09-23 19:15:20 eisen Exp $ */
/* This is the include file for echo */
/*! \file */


#ifndef __ECHO__H__
#define __ECHO__H__

#include "ffs.h"

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#if defined(FUNCPROTO) || defined(__STDC__) || defined(__cplusplus) || defined(c_plusplus)
#ifndef ARGS
#define ARGS(args) args
#endif
#else
#ifndef ARGS
#define ARGS(args) (/*args*/)
#endif
#endif

/* Channel functions */

/*!
 * EControlContext is the root of control flow and event handling in 
 * an ECho program.  
 * 
 * EControlContext is an opaque handle.
 */
typedef struct _EControlContext *EControlContext;

/*!
 * EChannel defined the space in which an event propogates.  
 * I.E. events are submitted to a particular channel and are 
 * delivered to sinks subscribed to that channel.
 * 
 * EChannel is an opaque handle.
 */
typedef struct _EChannel *EChannel;

/*!
 * An ECSourceHandle is associated with a particular channel and 
 * represents a capability to submit events into that channel.
 * 
 * ECSourceHandle is an opaque handle.
 */
typedef struct _ECSourceHandle *ECSourceHandle;

/*!
 * An ECSsinkHandle is associated with a particular channel and 
 * is returned as a result of subscribing a sink.  It can be used to 
 * cancel the subscription, or to "pull" an event for pull channels.
 * 
 * ECSinkHandle is an opaque handle.
 */
typedef struct _ECSinkHandle *ECSinkHandle;

/*!
 * An ECEventQueueList is a queue structure to hold events.  It is used in 
 * ECho multi-subscribe functionality
 * 
 * ECEventQueueList is an opaque handle.
 */
typedef struct _ECMultiEventQueue *ECEventQueueList;

/*!
 * An ECEventRec is a queue element associated with a single event.  
 * It is used in the ECho multi-subscribe functionality.
 * 
 * ECEventRec is an opaque handle.
 */
typedef struct _ECEventRec *ECEventRec;

/*!
 * ECHandlerFunction is the prototype for non-typed event handlers.
 * Non-typed events are simple variable-length blocks of data.  ECho 
 * allows functions matching this prototype to be registered as handlers 
 * with ECsink_subscribe and ECsink_subscribe_context.
 * 
 * \param event A pointer to the base of the incoming event data as void*.
 * \param length The length of the incoming event data.
 * \param client_data This value is the same client_data value that was
 * supplied in the ECsink_subscribe() call.  It is not interpreted by ECho,
 * but instead can be used to maintain some application context.
 * \param event_attrs The attributes (set of name/value pairs) that this 
 * event was delivered with.  These are determined by the ECho, CM and the 
 * underlying CM transport and may include those attrs specified in some 
 * variant of the ECSubmit() call when the event was submitted.
 */
typedef void (*ECHandlerFunction) ARGS((void *event, int length,
					void *client_data, attr_list event_attrs));

/*!
 * ECTypedHandlerFunction is the prototype for typed event handlers.
 * Typed events are delivered as application-structured data (I.E. 
 * ECho does all the marshalling and unmarshalling).  ECho 
 * allows functions matching this prototype to be registered as handlers 
 * with ECsink_typed_subscribe and ECsink_typed_subscribe_context.
 * 
 * \param event A pointer to the base of the incoming event data as void*.
 * The incoming data is formatted to match the fields of specified 
 * in the typed_subscribe() call.
 * \param client_data This value is the same client_data value that was
 * supplied in the ECsink_typed_subscribe() call.  It is not interpreted by 
 * ECho, but instead can be used to maintain some application context.
 * \param event_attrs The attributes (set of name/value pairs) that this 
 * event was delivered with.  These are determined by the ECho, CM and the 
 * underlying CM transport and may include those attrs specified in some 
 * variant of the ECSubmit() call when the event was submitted.
 */
typedef void (*ECTypedHandlerFunction) ARGS((void *event, void *client_data, attr_list event_attrs));

/*!
 * ECMultiHandlerFunction is the prototype for event handlers associated 
 * with multiple channels.  The events which are available are accessed 
 * through the event_queues list.  Events are *not* automatically dequeued 
 * and can be inspected in situ.
 * Typed events are delivered as application-structured data (I.E. 
 * ECho does all the marshalling and unmarshalling).  ECho 
 * allows functions matching this prototype to be registered as handlers 
 * with ECsink_multi_subscribe().
 * 
 * \param event_queues A list of queue structures holding pending events.
 * \param client_data This value is the same client_data value that was
 * supplied in the ECsink_multi_subscribe() call.  It is not interpreted by 
 * ECho, but instead can be used to maintain some application context.
 */
typedef void (*ECMultiHandlerFunction) ARGS((ECEventQueueList event_queues, 
					     void *client_data));

/*!
 * ECSubscribeFunction is the prototype for handlers associated with 
 * sink subscription occurrences.
 * 
 * \param subscribe 0 for sink subscribe occurrences, 1 for sink 
 * unsubscribe occurrences. 
 * \param subscriber_count An appoximation of the number of sinks associated with this channel.
 * \param client_data This value is the same client_data value that was
 * supplied in the EChannel_subscribe_function() call.  It is not interpreted
 * by ECho, but instead can be used to maintain some application context.
 */
typedef void (*ECSubscribeFunction) ARGS((int subscribe, int subscriber_count,
					  void *client_data));

/*!
 * EventFreeFunction is the prototype for a callback function to support 
 * application-specific memory management.
 * 
 * The EventFreeFunction prototype is used in the ECsubmit_general* variants. 
 * It specifies a callbacks to be used when ECho is done with a 
 * particular event.   (I.E. All ECho processing is finished and the event 
 * memory can be free'd or reused by the application.)
 * \param event_data  The base address of the event or vector submitted.
 */
typedef void (*EventFreeFunction) ARGS((void *event_data));

/*!
 * ECRequestHandlerFunction is the prototype for an event-pull mechanism 
 * in pull-style event channels.
 * Pull event channels operate with passive sources and active sinks (rather 
 * than the more standard active sources and passive sinks).  When a 
 * ECRequestHandlerFunction is registered as a data-producing callback, it
 * is called by ECho whenever a sink tries to pull data out of the channel.
 * The response by the application should be to submit data to the 
 * supplied source handle.
 * \param handle  The source handle to which data should be submitted.
 * \param client_data This value is the same client_data value that was
 * supplied in the ECPsource_subscribe() call.  It is not interpreted
 * by ECho, but instead can be used to maintain some application context.
 */
typedef void (*ECRequestHandlerFunction) ARGS((ECSourceHandle handle, 
					       void *client_data));

/*!\var ECMultiSubscribeList
 * The ECMultiSubscribeList data structure is used to specify the set 
 * of channels, field lists and subformat lists in a multi-subscribe 
 * situation.
 */

typedef struct _CMformat_list {
    /*! the name to be associated with this structure */
    char *format_name;
    /*! the PBIO-style list of fields within this structure */
    FMFieldList field_list;
} CMFormatRec, *CMFormatList;

typedef struct {
    EChannel channel;	/*! a channel to which we subscribe as a sink */
    FMFieldList field_list;	/*! the field list for that channel */
    CMFormatList subformat_list;/*! the subformat list for that field list */
}ECMultiSubscribeList, *ECMultiSubscribePtr;
/*!\var ECMultiSubscribePtr
 * A pointer to a ECMultiSubscribeList data structure, used to specify the set 
 * of channels, field lists and subformat lists in a multi-subscribe 
 * situation.
 */

/* supposed to be a quick fix for pipegl. 
 *Major question: How apps identify a connection 
for multiple sources/sinks channel, or shall we 
hide this and let echo/cm do automatical balancing 
for multiple socket connections of a specific channel?
* I.E. we should get rid of this ASAP.
*/
/*!
 * return the attribute list associated with a channel.
 *
 * This is actually an ugly hack.  There are no attributes associated 
 * with a channel (except for those it was created with).  This returns 
 * the attributes associated with the first connection associated with 
 * the CM associated with the channel.  This *might* have something to 
 * do with how events are transported, IFF the first thing you do after
 * ECho_CM_Init() is to open this particular channel.  Otherwise, all 
 * bets are off...
 *
 * \param chan  The channel for which the attribute list is returned.
 */
extern attr_list 
EChannel_get_attrs ARGS((EChannel chan));

/*!
 * initialize ECho
 * 
 * This is used to create an EControlContext and initialize ECho.  The 
 * created EControlContext is associated with the CManager.  Generally 
 * incoming events are handled after the network is serviced.  Bulk 
 * servicing may mean that event handler calls are batched.
 *
 * \param cm  The CManager with which to associated the EControlContext.
 */
extern EControlContext ECho_CM_init ARGS((CManager cm));

/*!
 * create a non-CM EControlContext
 * 
 * This is used to create an EControlContext that is *not* associated 
 * with a CManager.  This is mainly used to force event handlers to be called
 * by a specific thread.
 *
 */
extern EControlContext EContext_create();

/*!
 * handle pending network messages and ECho events on an EControlContext
 * 
 * This call handles some or all currently items and then returns control
 *
 * \param ec The EControlContext on which to handle items.
 */
extern void EControl_poll ARGS((EControlContext ec));

/*!
 * continuously handle network messages and ECho events on a EControlContext
 * 
 * This call handles actions associated with a EControlContext continuously, 
 * returning only if the EControlContext is shut down.
 *
 * \param ec The EControlContext on which to handle items.
 */
extern void EControl_run ARGS((EControlContext ec));

/*!
 * create an ECho event channel.
 * 
 * \param ec The EControlContext with which to associate the channel.  This 
 * ec must be associated with some CManager.
 * \return An valid EChannel pointer that can be used in subsequent calls.
 */
extern EChannel
 EChannel_create ARGS((EControlContext ec));

/*!
 * create an ECho event channel with particular attributes.
 * 
 * Attribute handling is not yet fully defined in ECho, but the attributes 
 * specified here will be passed to the CM-level connection creation calls.
 * ECho interprets some attributes.  In particular, the ECHO_EVENT_TRANSPORT
 * attribute is used to specify the name of a CM-level transport to be used 
 * for event (not control message) transmission on this channel.  Additionally,
 * the ECHO_EVENT_NETWORK attribute specifies a "postfix" which will be
 * appended to hostnames on contact addresses.  Essentially this activates
 * the CM_NETWORK_POSTFIX attribute for the event connections.
 * \param ec The EControlContext with which to associate the channel.  This 
 * ec must be associated with some CManager.
 * \param attrs The attribute list to associate with the channel.
 * \return An valid EChannel pointer that can be used in subsequent calls.
 */
extern EChannel
 EChannel_create_attr ARGS((EControlContext ec, attr_list attrs));

/*!
 * create a typed ECho event channel.
 * 
 * Typed echo channels are used to transport application-specified data
 * types.  The data in the channel contains (at least) the fields specified in
 * the field_list.  The subformat_list should contain the transitive closure of
 * the subformats that are used, specifying the name and field list for
 * each
 * \param ec The EControlContext with which to associate the channel.  This 
 * ec must be associated with some CManager.
 * \param field_list The PBIO-style list of fields that define the channel
 * content.  (Only the names and types are essential, the sizes and offsets
 * will vary with any actual data and are largely ignored here.)
 * \param subformat_list The transitive closure of subformats used in the
 * field list.
 * \return An valid EChannel pointer that can be used in subsequent calls.
 */
extern EChannel
EChannel_typed_create ARGS((EControlContext ec, FMFieldList field_list,
			    CMFormatList subformat_list));

/*!
 * create a typed ECho event channel with particular attributes.
 *
 * This combines the characteristics of EChannel_create_attr() and
 * EChannel_typed_create().
 *
 * \param ec The EControlContext with which to associate the channel.  This 
 * ec must be associated with some CManager.
 * \param field_list The PBIO-style list of fields that define the channel
 * content.  (Only the names and types are essential, the sizes and offsets
 * will vary with any actual data and are largely ignored here.)
 * \param subformat_list The transitive closure of subformats used in the
 * field list.
 * \param attrs The attribute list to associate with the channel.
 * \return An valid EChannel pointer that can be used in subsequent calls.
 */
extern EChannel
 EChannel_typed_create_attr ARGS((EControlContext ec, FMFieldList field_list,
				  CMFormatList subformat_list, attr_list attrs));

/*!
 * Destroy the local components of an event channel.
 *
 * Event channels are essentially distributed entities, with components in
 * every process where they are used.  This call deallocates all local
 * components, cancels subscriptions, etc.  It is <b>NOT</b> a synchronous
 * distributed call, therefore may produce race conditions and should not
 * really be used except in final shutdown.
 *
 * \param chan The channel to destroy.
 */
extern void
 EChannel_destroy ARGS((EChannel chan));

/*!
 * return the global name of a channel.
 *
 * The textual global name of a channel is used to access a channel
 * remotely.  The typical use of ECho involves creating a channel, using
 * ECglobal_id() to get it's name, and then using that name in another
 * process to open or otherwise access the channel.  ECho does not directly
 * provide for the transmission of names, leaving that to the application.
 *
 * \param chan The channel for which to produce a global name.
 * \returns a char* string representing the global ID.  The application is
 * responsible for deallocating the string.
 */
extern char *ECglobal_id ARGS((EChannel chan));

/*!
 * Open a channel for local use using a global ID.
 *
 * This call is used to open (create a local component for) a channel that
 * was created elsewhere.
 * \param ec The EControlContext with which to associate the channel.  This 
 * ec must be associated with some CManager.
 * \param global_name  The global id of the channel to open, as returned by 
 * ECglobal_id(). 
 * \return An valid EChannel pointer that can be used in subsequent calls.
 */
extern EChannel
EChannel_open ARGS((EControlContext ec, char *global_name));

/*!
 * subscribe a subroutine as a untyped handler for incoming events on a channel
 *
 * Incoming data is delivered as a contiguous block of bytes.
 *
 * \param chan The channel for which this handler will act as a data sink.
 * \param func The subroutine that accepts the data.  It should match the
 * ECHandlerFunction profile.
 * \param client_data This void* value is not interpreted by ECho and is
 * used to maintain some application context.
 * \returns An ECSinkHandle that can be used in a later
 * ECcancel_sink_subscribe() call.  It will be NULL if the channel could not
 * be opened.
 */
extern ECSinkHandle
ECsink_subscribe ARGS((EChannel chan, ECHandlerFunction func, void *client_data));

/*!
 * subscribe a subroutine as a untyped handler in a particular context.
 *
 * Incoming data is delivered as a contiguous block of bytes.
 *
 * \param chan The channel for which this handler will act as a data sink.
 * \param func The subroutine that accepts the data.  It should match the
 * ECHandlerFunction profile.
 * \param client_data This void* value is not interpreted by ECho and is
 * used to maintain some application context.
 * thread associated with that context <i>may</i> run the handler.  (For
 * locally submitted events, the submitting thread will likely run it.)
 * \param ec  The EControlContext with which to associate the handler.
 * \returns An ECSinkHandle that can be used in a later
 * ECcancel_sink_subscribe() call.
 */
extern ECSinkHandle
 ECsink_subscribe_context ARGS((EChannel chan, ECHandlerFunction func,
				void *client_data, EControlContext ec));

/*!
 * subscribe a subroutine as a typed handler for incoming events on a channel
 *
 * Incoming data is delivered as structured application data type matching
 * that specified by the field list.  The set of fields in this call must be
 * equivalent to, or a subset of, the fields specified in the
 * EChannel_typed_create() call.  (Field order, offsets and sizes may vary
 * from that specified in EChannel_create, but must match the exact data
 * layout the handler expects.)
 *
 * \param chan The channel for which this handler will act as a data sink.
 * \param func The subroutine that accepts the data.  It should match the
 * ECHandlerFunction profile.
 * \param field_list The PBIO-style list of fields that define the data this
 * handler expects. content.
 * \param subformat_list The transitive closure of subformats used in the
 * field list.
 * \param client_data This void* value is not interpreted by ECho and is
 * used to maintain some application context.
 * \returns An ECSinkHandle that can be used in a later
 * ECcancel_sink_subscribe() call.  It will be NULL if the channel could not
 * be opened.
 */
extern ECSinkHandle
 ECsink_typed_subscribe ARGS((EChannel chan, FMFieldList field_list,
			      CMFormatList subformat_list,
			      ECTypedHandlerFunction func, void *client_data));

/*!
 * subscribe a subroutine as a typed handler in a particular context
 *
 * Incoming data is delivered as structured application data type matching
 * that specified by the field list.  The set of fields in this call must be
 * equivalent to, or a subset of, the fields specified in the
 * EChannel_typed_create() call.  (Field order, offsets and sizes may vary
 * from that specified in EChannel_create, but must match the exact data
 * layout the handler expects.)
 *
 * \param chan The channel for which this handler will act as a data sink.
 * \param func The subroutine that accepts the data.  It should match the
 * ECHandlerFunction profile.
 * \param field_list The PBIO-style list of fields that define the data this
 * handler expects. content.
 * \param subformat_list The transitive closure of subformats used in the
 * field list.
 * \param ec  The EControlContext with which to associate the handler.
 * \param client_data This void* value is not interpreted by ECho and is
 * used to maintain some application context.
 * \param ec  The EControlContext with which to associate the handler.  The
 * \returns An ECSinkHandle that can be used in a later
 * ECcancel_sink_subscribe() call.  It will be NULL if the channel could not
 * be opened.
 */
extern ECSinkHandle
 ECsink_typed_subscribe_context ARGS((EChannel chan, FMFieldList field_list,
				      CMFormatList subformat_list,
				      ECTypedHandlerFunction func,
				      void *client_data, EControlContext ec));

/*!
 * subscribe a subroutine as handler for multiple channels
 *
 * Multisubscribe handlers are called whenever data arrives on any channel.
 *
 * \param sink_specs The list of channels to be subscribed to, including
 * field lists and subformat lists for each.
 * \param ec  The EControlContext with which to associate the handler.
 * \param func The subroutine that accepts the data.  It should match the
 * ECHandlerFunction profile.
 * \param client_data This void* value is not interpreted by ECho and is
 * used to maintain some application context.
 * \returns An ECSinkHandle that can be used in a later
 * ECcancel_sink_subscribe() call.  It will be NULL if the channel could not
 * be opened.
 */
extern ECSinkHandle
 ECsink_multi_subscribe ARGS((ECMultiSubscribePtr sink_specs,
			      EControlContext ec,
			      ECMultiHandlerFunction func, 
			      void *client_data));

/*!
 * cancel a sink subscribe (unsubscribe)
 *
 * This call deallocates the sink handle and cancels the subscribe (so 
 * that the function is no longer called in response to incoming events).
 * 
 * \param handle The handle to be cancelled.
 */
extern void
ECcancel_sink_subscribe ARGS((ECSinkHandle handle));

/*!
 * dump an incoming event as XML
 *
 * this function operates on <b>unmarshalled</b> output, such as is 
 * delivered to a typed sink.  The XML is dumped to standard output.
 * 
 * \param chan the channel whose type defines the data
 * \param event a pointer to the base of the data
 */
extern void ECdump_typed_event_as_XML ARGS((EChannel chan, void* event));

/*!
 * dump an encoded incoming event as XML
 *
 * this function operates on <b>PBIO marshalled</b> data, such as might be 
 * delivered to an untyped sink on a typed channel.  The XML is dumped to 
 * standard output.
 * 
 * \param chan the channel whose type defines the data
 * \param event a pointer to the base of the data
 */
extern void ECdump_event_as_XML ARGS((EChannel chan, void* event));

/*!
 * create an untyped source handle for a channel
 *
 * The source handle can be used to submit events to the channel.  The
 * untyped source subscribe is used only on untyped channels.
 * 
 * \param chan The channel to which a source will be created
 * \returns an ECSourceHandle to be used in ECsubmit_event() calls.
 */
extern ECSourceHandle
ECsource_subscribe ARGS((EChannel chan));

/*!
 * create an typed source handle for a channel
 *
 * The source handle can be used to submit events to the channel.  The
 * typed source subscribe is used on typed channels.  Data is submitted as
 * structured application data.  The field_list and format_list must
 * exactly match the layout of that data that will be submitted.  The set of
 * fields in this call must be equivalent to, or a <b>superset</b> of, the
 * fields specified in the EChannel_typed_create() call (I.E. you can
 * submit more data than the channel expects, but not less).  (Field order,
 * offsets and sizes may vary from that specified in EChannel_create.)
 * 
 * \param chan The channel to which a source will be created
 * \param field_list The PBIO-style list of fields that define the data this
 * handler expects. content.
 * \param subformat_list The transitive closure of subformats used in the
 * field list.
 * \returns an ECSourceHandle to be used in ECsubmit_typed_event() calls.
 */
extern ECSourceHandle
ECsource_typed_subscribe ARGS((EChannel chan, FMFieldList field_list,
				CMFormatList subformat_list));

/*!
 * create an typed source handle for a channel with attributes
 *
 * The source handle can be used to submit events to the channel.  The
 * typed source subscribe is used on typed channels.  Data is submitted as
 * structured application data.  The field_list and format_list must
 * exactly match the layout of that data that will be submitted.  The set of
 * fields in this call must be equivalent to, or a <b>superset</b> of, the
 * fields specified in the EChannel_typed_create() call (I.E. you can
 * submit more data than the channel expects, but not less).  (Field order,
 * offsets and sizes may vary from that specified in EChannel_create.)
 * 
 * \param chan The channel to which a source will be created
 * \param field_list The PBIO-style list of fields that define the data this
 * handler expects.
 * \param subformat_list The transitive closure of subformats used in the
 * field list.
 * \param attrs  This parameter is currently ignored, but may at some point
 * influence the submission of events.
 * \returns an ECSourceHandle to be used in ECsubmit_typed_event() calls.
 */
extern ECSourceHandle
ECsource_typed_subscribe_attr ARGS((EChannel chan, FMFieldList field_list,
				     CMFormatList subformat_list,
				     attr_list attrs));

/*!
 * cancel a source subscribe (unsubscribe)
 *
 * This call deallocates the source handle and cancels the subscribe (so 
 * that it can no longer be used to submit events).
 * 
 * \param handle The handle to be cancelled.
 */
extern void
ECcancel_source_subscribe ARGS((ECSourceHandle handle));

/*!
 * request a callback upon subscribe state change
 *
 * This function is used to register a subscribe callback function, 
 * which will be invoked wherever the local channel is notified of a 
 * change in the subscriber state of a function.  (Local changes do not 
 * always require global notification, so some changes generate no 
 * notification, such as adding a sink in a process which already has 
 * an existing sink.)
 *
 * \param chan the channel to monitor
 * \param func the callback function to be invoked
 * \param client_data This void* value is not interpreted by ECho and is
 * used to maintain some application context.
 */
extern void
EChannel_subscribe_handler ARGS((EChannel chan, ECSubscribeFunction func,
				 void *client_data));
/*!
 * return an appoximation of the number if sinks on a channel
 *
 * \param handle a source handle to the channel in question
 * \returns an approximation of the number of sinks on this channel
 * (The appoximation is the number of local sinks plus the count of 
 * outbound communication links that a submitted event would be sent upon.)
 */
extern int EChas_sinks ARGS((ECSourceHandle handle));

/*!
 * return the CMConnections that would be used for outbound events 
 * for a particular source
 *
 * \param handle a source handle to the channel in question
 * \returns A null-terminated list of CMConnection values that would be 
 * used for outbound events submitted to a source.  The caller is 
 * responsible for freeing the list.
 */
extern CMConnection *ECget_outbound_conns ARGS((ECSourceHandle handle));

/*!
 * return an appoximation of the number if sinks on a channel
 *
 * \param handle a source handle to the channel in question
 * \returns an approximation of the number of sinks on this channel
 * (The appoximation is the number of local sinks plus the count of 
 * outbound communication links that a submitted event would be sent upon.)
 */
extern int EChas_sinks ARGS((ECSourceHandle handle));

/*!
 * return the field list associated with an open typed channel
 *
 * This is useful for discovery without a priori knowledge
 *
 * \param chan the channel
 * \returns a PBIO-style field_list which defines the type associated with the channel.
 */
extern FMFieldList EChannel_get_field_list ARGS((EChannel chan));
/*!
 * return the subformat list associated with an open typed channel
 *
 * This is useful for discovery without a priori knowledge
 *
 * \param chan the channel
 * \returns a PBIO-style subformat_list which defines the type associated with the channel.
 */
extern CMFormatList EChannel_get_format_list ARGS((EChannel chan));

/*!
 * submit an untyped event
 *
 * \param handle The source handle through which to submit the event
 * \param event The base address of the event data to submit, a block 
 * of contiguous bytes
 * \param event_length The length of the event data
 */ 
extern void
ECsubmit_event ARGS((ECSourceHandle handle, void *event, int event_length));

/*!
 * submit a typed event
 *
 * \param handle The source handle through which to submit the event
 * \param event The base address of the event data structure to submit, the
 * event data is structured as given in the field_list of the source subscribe.
 */ 
extern void
ECsubmit_typed_event ARGS((ECSourceHandle handle, void *event));

/*!
 * submit a typed event with attributes
 *
 * \param handle The source handle through which to submit the event
 * \param event The base address of the event data structure to submit, the
 * event data is structured as given in the field_list of the source
 * subscribe.
 * \param attrs The attributes are supplied to an CM-level write calls that
 * result from this event submission.  The one attribute that is directly
 * interpreted by ECho at this point is ECHO_USE_EVENT_TRANSPORT.  This is a
 * boolean that controls whether or not the event is submitted to the
 * separate event transport (as specified by ECHO_EVENT_TRANSPORT).  All
 * attributes specified here are delivered to the handlers on the sink side,
 * be they local or remote.
 */ 
extern void
ECsubmit_typed_event_attr ARGS((ECSourceHandle handle, void *event, 
				attr_list attrs));

/*!
 * submit an untyped event with data in a non-contiguous vector
 *
 * \param handle The source handle through which to submit the event
 * \param eventv the event data, represented by a byte-vector.  I.E. the
 * vector specifies a set of buffers and lengths, that if made contiguous
 * would represent the complete event data.  This call is used to avoid copy
 * overheads in making the data contiguous.
 */ 
extern void
ECsubmit_eventV ARGS((ECSourceHandle handle, FFSEncodeVector eventv));

/*!
 * submit an untyped event with data in a non-contiguous vector
 *
 * \param handle The source handle through which to submit the event
 * \param event The base address of the event data to submit, a block 
 * of contiguous bytes
 * \param event_length The length of the event data
 * \param free_func the deallocation callback that is run when ECho is
 * finished with the event data.
 * \param attrs The attributes are supplied to an CM-level write calls that
 * result from this event submission.  The one attribute that is directly
 * interpreted by ECho at this point is ECHO_USE_EVENT_TRANSPORT.  This is a
 * boolean that controls whether or not the event is submitted to the
 * separate event transport (as specified by ECHO_EVENT_TRANSPORT).  All
 * attributes specified here are delivered to the handlers on the sink side,
 * be they local or remote.
 */ 
extern void
ECsubmit_general_event ARGS((ECSourceHandle handle, void *event,
			     int event_length, EventFreeFunction free_func, 
			     attr_list attrs));

/*!
 * submit an typed event with data in a non-contiguous vector with free
 * function and attributes
 *
 * The normal event submission semantics are that when the ECsubmit call
 * returns, ECho is finished with the data and it can be overwritten by the
 * application.  The *general*submit routines return immediately, allowing
 * ECho to retain control of the data.  This routine has an additional
 * parameter to allow the application to specify an appropriate deallocation
 * routine for the event data.  This allows the event system to keep the
 * event data until all local handers have run and then deallocate it.  The
 * application should not reference the event data after it has been
 * submitted until the deallocation routine is called.
 *
 * \param handle The source handle through which to submit the event
 * \param event The base address of the event data structure to submit, the
 * event data is structured as given in the field_list of the source
 * subscribe.
 * \param free_func the deallocation callback that is run when ECho is
 * finished with the event data.
 * \param attrs The attributes are supplied to an CM-level write calls that
 * result from this event submission.  The one attribute that is directly
 * interpreted by ECho at this point is ECHO_USE_EVENT_TRANSPORT.  This is a
 * boolean that controls whether or not the event is submitted to the
 * separate event transport (as specified by ECHO_EVENT_TRANSPORT).  All
 * attributes specified here are delivered to the handlers on the sink side,
 * be they local or remote.
 */ 
extern void
ECsubmit_general_typed_event ARGS((ECSourceHandle handle, void *event,
				   EventFreeFunction free_func, 
				   attr_list attrs));

/*!
 * submit an untyped event with data in a non-contiguous vector with free
 * function
 *
 * The normal event submission semantics are that when the ECsubmit call
 * returns, ECho is finished with the data and it can be overwritten by the
 * application.  The *general*submit routines return immediately, allowing
 * ECho to retain control of the data.  This routine has an additional
 * parameter to allow the application to specify an appropriate deallocation
 * routine for the event data.  This allows the event system to keep the
 * event data until all local handers have run and then deallocate it.  The
 * application should not reference the event data after it has been
 * submitted until the deallocation routine is called.
 *
 * \param handle The source handle through which to submit the event
 * \param eventV the event data, represented by a byte-vector.  I.E. the
 * vector specifies a set of buffers and lengths, that if made contiguous
 * would represent the complete event data.  This call is used to avoid copy
 * overheads in making the data contiguous.
 * \param free_func the deallocation callback that is run when ECho is
 * finished with the event data.
 */ 
extern void
ECsubmit_general_eventV ARGS((ECSourceHandle handle, FFSEncodeVector eventV,
			      EventFreeFunction free_func));

/*!
 * take control of the memory occupied by an event
 *
 * Normally, ECho retains control of event memory and it is guaranteed to
 * remain valid only for the duration of the sink handler subroutine.  If
 * the data needs to be retained longer, applications can take over control
 * of the memory by calling ECtake_event_buffer() from within the handler.
 * ECho will then not overwrite or reuse the buffer until it is returned.
 * 
 * \param ec The EControlContext that controls the running handler.
 * \param event The base address of the event that the application wants
 * control over.
 */
extern int
ECtake_event_buffer ARGS((EControlContext ec, void *event));

/*!
 * return control of the memory occupied by an event
 *
 * This restores ECho's control over event memory.
 * 
 * \param ec The EControlContext that controlled the handler that the event
 * came from.
 * \param event The base address of the event that the application is
 * returning. 
 */
extern void
ECreturn_event_buffer ARGS((EControlContext ec, void *event));

/**
 * \defgroup EDC Derived event channel creation functions
 */
/*@{*/
/*!
 * derive a new event channel based on a filter function
 *
 * The new channel is associated with the original channel (specified by the
 * global ID in the chan_id parameter) through the event_filter function.
 * When an event is submitted to the original channel, the event_filter
 * function is run.  If it returns TRUE (non-zero), the event is also
 * submitted to the derived channel.  The new channel takes the type of 
 * the original channel.  The event is available as parameter "input" to 
 * the filter function.
 *
 * \param ec The EControlContext with which to associate the channel.  This 
 * ec must be associated with some CManager.
 * \param chan_id  The global id of base channel from which the new
 * channel is derived, as returned by ECglobal_id(). 
 * \param event_filter The filter function, expressed as textual ECL code.
 * \return An valid EChannel pointer that can be used in subsequent calls.
 */
extern EChannel
EChannel_derive ARGS((EControlContext ec, char *chan_id, char *event_filter));

/*!
 * derive a new event channel based on a transforming filter
 *
 * The new channel is associated with the original channel (specified by the
 * global ID in the chan_id parameter) through the event_filter function.
 * When an event is submitted to the original channel, the event_filter
 * function is run.  If it returns TRUE (non-zero), the event is also
 * submitted to the derived channel.  The new channel has the type specified 
 * by the field_list and subformat_list parameters.  The input event is 
 * available as parameter "input" to the transformation/filter function.  
 * The output event is avilable as parameter "output" to the function and 
 * <b>must</b> be explicitly initialized in the function.
 *
 * \param ec The EControlContext with which to associate the channel.  This 
 * ec must be associated with some CManager.
 * \param chan_id  The global id of base channel from which the new
 * channel is derived, as returned by ECglobal_id(). 
 * \param event_filter The filter function, expressed as textual ECL code.
 * \param field_list The PBIO-style list of fields that define the channel
 * content.  (Only the names and types are essential, the sizes and offsets
 * will vary with any actual data and are largely ignored here.)
 * \param subformat_list The transitive closure of subformats used in the
 * field list.
 * \return An valid EChannel pointer that can be used in subsequent calls.
 */
extern EChannel
EChannel_typed_derive ARGS((EControlContext ec, char *chan_id, char *event_filter,
			    FMFieldList field_list, CMFormatList subformat_list));

/*!\var ECdata_struct
 * The ECdata_struct data structure is used to specify the field list,
 * subformat lists and initial value for stable data in a derivation.
 */
/*!\var ECdata_spec
 * A pointer to an ECdata_struct data structure, used to specify the field list,
 * subformat lists and initial value for stable data in a derivation.
 */
typedef struct {
    FMFieldList data_field_list;	/*! the field list for that channel */
    CMFormatList data_subformat_list;/*! the subformat list for that field list */
    void *initial_value; /*! a pointer to the initial value for the stable data*/
}ECdata_struct, *ECdata_spec;

/*!
 * derive a new event channel based on a filter function with associated stable data
 *
 * The new channel is associated with the original channel (specified by the
 * global ID in the chan_id parameter) through the event_filter function.
 * When an event is submitted to the original channel, the event_filter
 * function is run.  If it returns TRUE (non-zero), the event is also
 * submitted to the derived channel.  The new channel takes the type of 
 * the original channel.  The event is available as parameter "input" to 
 * the filter function.  The filter function also has available a parameter 
 * "data", defined by the ECdata_spec value in data_spec.  This data is 
 * replicated on all locations where the filter function exists and can be 
 * updated with EChannel_data_update().  Its field list, subformat lists 
 * and initial value are defined by data_spec.
 *
 * \param ec The EControlContext with which to associate the channel.  This 
 * ec must be associated with some CManager.
 * \param chan_id  The global id of base channel from which the new
 * channel is derived, as returned by ECglobal_id(). 
 * \param event_filter The filter function, expressed as textual ECL code.
 * \param data_spec data structure which defines the field list, subformat 
 * lists and initial value of the stable data
 * \return An valid EChannel pointer that can be used in subsequent calls.
 */
extern EChannel 
EChannel_derive_data ARGS((EControlContext ec, char *chan_id, 
			   char *event_filter,
			   ECdata_spec data_spec));

/*!
 * derive a new event channel based on a transforming filter
 *
 * The new channel is associated with the original channel (specified by the
 * global ID in the chan_id parameter) through the event_filter function.
 * When an event is submitted to the original channel, the event_filter
 * function is run.  If it returns TRUE (non-zero), the event is also
 * submitted to the derived channel.  The new channel has the type specified 
 * by the field_list and subformat_list parameters.  The input event is 
 * available as parameter "input" to the transformation/filter function.  
 * The output event is avilable as parameter "output" to the function and 
 * <b>must</b> be explicitly initialized in the function.  The filter 
 * function also has available a parameter  
 * "data", defined by the ECdata_spec value in data_spec.  This data is 
 * replicated on all locations where the filter function exists and can be 
 * updated with EChannel_data_update().  Its field list, subformat lists 
 * and initial value are defined by data_spec.
 *
 * \param ec The EControlContext with which to associate the channel.  This 
 * ec must be associated with some CManager.
 * \param chan_id  The global id of base channel from which the new
 * channel is derived, as returned by ECglobal_id(). 
 * \param event_filter The filter function, expressed as textual ECL code.
 * \param field_list The PBIO-style list of fields that define the channel
 * content.  (Only the names and types are essential, the sizes and offsets
 * will vary with any actual data and are largely ignored here.)
 * \param subformat_list The transitive closure of subformats used in the
 * field list.
 * \param data_spec data structure which defines the field list, subformat 
 * lists and initial value of the stable data
 * \return An valid EChannel pointer that can be used in subsequent calls.
 */
extern EChannel 
EChannel_typed_derive_data ARGS((EControlContext ec, char *chan_id,
				 char *event_filter,
				 FMFieldList field_list, 
				 CMFormatList subformat_list,
				 ECdata_spec data_spec));

/*@}*/
#ifdef THIRD_PARTY_DERIVATION
extern EChannel
EChannel_create_filtered ARGS((EControlContext ec, char *chan_id,
			       char *event_filter));

extern EChannel
EChannel_create_transformed ARGS((EControlContext ec, char *chan_id, 
				  char *event_xform, FMFieldList field_list,
				  CMFormatList format_list));

extern char *
EChannel_remote_filter ARGS((EControlContext ec, char *remote_contact_list, 
			     char *src_chan_id, char *event_filter));

extern char *
EChannel_remote_transform ARGS((EControlContext ec, 
				char *remote_contact_list, 
				char *src_chan_id, char *event_filter, 
				FMFieldList field_list, 
				CMFormatList format_list));

#endif
extern void
ECrequest_event ARGS((ECSinkHandle handle, int req_try));

extern ECSourceHandle
 ECPsource_subscribe ARGS((EChannel chan, 
			   ECRequestHandlerFunction func, void *client_data));

extern ECSourceHandle 
  ECPsource_typed_subscribe ARGS((EChannel chan, FMFieldList field_list, 
				  CMFormatList format_list, 
				  ECRequestHandlerFunction func, void *client_data));

extern ECSourceHandle 
ECPsource_typed_subscribe_attr ARGS((EChannel chan, FMFieldList field_list, 
				     CMFormatList format_list, 
				     ECRequestHandlerFunction func, 
				     void *client_data, attr_list attrs));

/**
 * \defgroup EPF Proto channel functions
 */
/*@{*/

/*!
 * ECproto is an opaque handle to a proto-channel, a set of passive resources 
 * from which a new channel can be created.
 * 
 * EChannel is an opaque handle.
 */
typedef struct _ECproto *ECproto;

#ifdef __ECL__H__
/*!
 * ECproto_create() creates a new proto-channel to be associated with the
 * passive resources (variables and functions) which have been registered
 * in the ecl_parse_context value.
 *
 * \param ec The EControlContext to be associated with this proto-channel
 * \param context The ecl_parse_context which defines its available resources.
 * \returns an ECproto handle that can be used in ECproto_id().
 */
extern ECproto ECproto_create ARGS((EControlContext ec, ecl_parse_context context));
#endif

/*!
 * return the global ID of a proto-channel
 *
 * \param proto_chan the proto-channel for which to return the ID
 * \returns a textual global ID.  This value must be free'd by the application.
 */
extern char *ECproto_id ARGS((ECproto proto_chan));

/*!
 * create a new channel from a passive proto-channel
 *
 * ECproto_derive_periodic specifies that a gather_function will be run 
 * with a stated periodicity at the location of the proto-channel.  This 
 * function can query the resources of the proto-channel to produce data 
 * which will be submitted to a newly created channel.  The gather function 
 * must fill in the non-initialized "output" parameter.
 *
 * \param ec the EControlContext with which to associate the new channel
 * \param proto_id The global ID of the proto-channel
 * \param gather_function textual ECL source for the gather function
 * \param field_list the field list that defined the output of the gather 
 * function
 * \param subformat_list The transitive closure of subformats used in the
 * field list.
 * \param microsec_period the period at which the function is to be run, 
 * expressed in microseconds.
 */
extern EChannel 
ECproto_derive_periodic ARGS((EControlContext ec, char *proto_id, 
			      char *gather_function,
			      FMFieldList field_list, CMFormatList subformat_list,
			      int microsec_period));

/**
 * \defgroup EDM functions for manipulating stable data associated with derivation functions
 */
/*@{*/

/*!
 * An ECDataHandle can be associated with channels which have been 
 * created through one of the channel derive with data calls.
 * It is used to update the stable "data" values associated with 
 * derivation functions.
 * 
 * ECDataHandle is an opaque handle.
 */
typedef struct _ECDataHandle *ECDataHandle;

/*!
 * obtain an update handle to a stable data value
 *
 * Data is updated as structured application data.  The field_list and 
 * format_list must exactly match the layout of that data that will 
 * be submited as an update.  The set of 
 * fields in this call must be equivalent to, or a <b>superset</b> of, the
 * fields specified in the EChannel_derive_data() call (I.E. you can
 * submit more data than the channel expects, but not less).
 *
 * \param channel The derived event channel with which the data is associated
 * \param data_field_list The PBIO-style list of fields that define the data 
 * to be submitted for update.
 * \param data_subformat_list The transitive closure of subformats used in the
 * field list.
 * \returns an ECDataHandle to be used in EChannel_data_update() calls.
 */
extern ECDataHandle 
EChannel_data_open ARGS((EChannel channel, FMFieldList data_field_list,
			 CMFormatList data_subformat_list));

/*!
 * update the stable data associated with derivation functions
 *
 * \param data_handle the ECDataHandle obtained from EChannel_data_open()
 * \param data the structured data with which to update the stable data.
 */
extern void 
EChannel_data_update ARGS((ECDataHandle data_handle, void *data));

/*!
 * shut down an ECDataHandle
 *
 * \param handle the ECDataHandle to shut down
 */
extern void
EChannel_data_close ARGS((ECDataHandle handle));
/*@}*/

/**
 * \defgroup EVQ Event Queue Manipulation Functions (for multisubscribe)
 */
/*@{*/

/*!
 * return the ECEventRec associated with a particular event
 * 
 *
 * \param queues  the set of event queues
 * \param queue_number the index of the queue from which to remove the event
 * \param event_number the index of the event to remove
 * \returns an ECEventRec
 */
extern ECEventRec
ECQget_event_rec ARGS((ECEventQueueList queues, int queue_number,
		       int event_number));

/*!
 * return the number of queues in a queue list
 *
 * \param queues  the list of event queues
 * \returns the number of queues in the list
 */
extern int
ECQCount ARGS((ECEventQueueList queues));

/*!
 * return the number of events in a queue
 *
 * \param queues  the set of event queues
 * \param queue the index of the queue from which to remove the event
 * \returns the number of events in the specified queue
 */
extern int
ECQEventCount ARGS((ECEventQueueList queues, int queue));

/*!
 * dequeues a particular event from a particular queue in a list of queues
 *
 * \param queues  the set of event queues
 * \param queue the index of the queue from which to remove the event
 * \param event_number the index of the event to remove
 */
extern void 
ECEvent_remove ARGS((ECEventQueueList queues, int queue,
                     int event_number));

/*!
 * Given an ECEventRec, return a pointer to the event data
 *
 * \param event_rec The event record 
 * \returns a pointer to the event data associated with that record
 */
extern void *
ECEvent_get_ptr ARGS((ECEventRec event_rec));
/*@}*/


#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
