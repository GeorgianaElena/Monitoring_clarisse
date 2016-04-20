#ifndef __CMRPC_H__
#define __CMRPC_H__

/*
 *
 *  $Id: cmrpc.h,v 1.26 2007-12-24 00:45:13 pmw Exp $
 *
 */
/*! \file */

#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>

#include "atl.h"

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


  /**
   * \defgroup server Server-side functions and structures
   */
  /*@{*/

  /*! Options struct for definers of RPC calls.
   */
typedef struct
{
  CManager cm;
  void* channel;
  unsigned long flags;
} *CMrpc_options, CMrpc_options_t;

  /*! Type of most RPC handler functions.
The in_param and out_param structures must be cast to user types
(which themselves must already have been registered) before use.
\param in_param_msg the input data to the handler.
\param out_param_msg the output data from the handler function.
\param options the options structure used by the handler.
  */
typedef void (*CMrpc_HandlerFunc) ARGS((void* in_param_msg, void* out_param_msg, CMrpc_options options));

  /*! Type of "one-way" functions, which are RPC calls for which the caller does not expect a reply.
\param in_param_msg the input data to the handler.
\param options the options structure used by the handler.
  */
typedef void (*CMrpc_OnewayHandlerFunc) ARGS((void* in_param_msg, CMrpc_options options));

  /*! Type of handler cleanup functions.  A handler cleanup function is run after the handler function has been executed and the data returned to the caller.  Typically, these free up temporary memory allocated by the handler or release other resources.
\param cm The CManager used by the handler.  The function needs access to this to release CM resources (such as memory buffers using take_buffer()).
\param out_msg The output message sent from the handler.  If this structure contained any memory that was dynamically allocated, it can be freed using this pointer.
   */
typedef void (*CMrpc_HandlerCleanupFunc) ARGS((CManager cm,
                                                void* out_msg));

  /*@{*/
  /*! Options manipulation for the CMrpc_options struct.  The set and clear functions are shortcuts to change the flags member of the struct.  call_cleanup_function indicates that there is a cleanup function for the handler that should be called after the handler completes and any data is returned to the caller.
  */
  /*@}*/
#define CMrpc_set_option(X,OPT) ((X)->flags |= (OPT))
#define CMrpc_clear_option(X,OPT) ((X)->flags &= ~(OPT))

#define CMrpc_call_cleanup_function (0x1)


/*
 * option flags for RPC calls
 */
#define CMrpc_async_call (0x1)


  /*! Used from server-side (i.e. CMrpc assumes that if you use this function in a process, you intend for that process to receive RPC requests). Registers the given function and, if desired, cleanup function as handlers for the given rpc name.  
\param cm The CManager in use.
\param rpc_name The RPC for which this handler is being registered.
\param in_desc An FMStructDescList describing the RPC input format.
\param out_desc An FMStructDescList describing the RPC output format.
\param func The handler function to be called by CMrpc.  Must not be NULL.
\param oneway_func A handler function to be called if a client makes a oneway RPC.  Specify NULL if not needed.
\param cleanup_func A cleanup function if one is needed for this RPC.  Specify NULL if not needed.
\return Returns 1 if successful, -1 otherwise (and sets CMrpc_errno).
\par CMrpc_errno values used:
EEXISTS if an RPC handler is already registered with the name rpc_name.

  */
extern int
CMrpc_register_rpc_handler ARGS((CManager cm,
				 const char* rpc_name,
				 FMStructDescList in_desc,
				 FMStructDescList out_desc,
				 CMrpc_HandlerFunc func,
				 CMrpc_OnewayHandlerFunc oneway_func,
				 CMrpc_HandlerCleanupFunc cleanup_func));

  /*! Removes the registration information for the named RPC.
\param rpc_name The RPC to remove.
\returns Returns 1 if successful, -1 otherwise (and sets CMrpc_errno).
\par CMrpc_errno values used:
ENOENT if the name doesn't match any registered RPC.
*/
extern int
CMrpc_unregister_rpc ARGS((const char* rpc_name));

  /*@}*/


  /**
   * \defgroup format Format-handling and other common items
   */
  /*@{*/

/*! The CMrpc errno.  Contains a CMrpc-specific error code from errno.h.
 */
extern int CMrpc_errno;


  /*! Client-side RPC registration function.
\param cm The CManager in use.
\param rpc_name The RPC for which this format is to be registered.
\param contact_list An attr_list with contact information for the RPC.
\param in_struct An FMStructDescList describing the RPC input format.
\param out_struct An FMStructDescList describing the RPC output format.
\return Returns 1 if successful, -1 if not (and sets CMrpc_errno).
\sa FFS (FMStructDescList) documentation.
  */
extern int
CMrpc_register_rpc_request ARGS((CManager cm,
				 const char* rpc_name,
				 attr_list contact_list,
				 FMStructDescList in_struct,
				 FMStructDescList out_struct));

  /*! Used to terminate RPC services and release all resources.*/
extern void
CMrpc_shutdown();

  /*@}*/

  /**
   * \defgroup client Client-side functions
   */
  /*@{*/

struct _CMrpc_ticket_struct;
  /*! An asynchronous RPC ticket that can be later "redeemed" to retrieve the results of the RPC.
   */
typedef struct _CMrpc_ticket_struct *CMrpc_ticket;


  /*! Client-side RPC call. Message formats and contact information for the RPC must be properly registered before calling this function. in_data and out_data must point to valid buffers corresponding to the registered message formats. If async is 1, the call is made asynchronously and the return value is a CMrpc_ticket that can be used to retrieve the results of the operation; otherwise, the call blocks until the results arrive and the return value is NULL. If async is 1 and the return value is NULL, an error occurred; check CMrpc_errno for an indication of the error.
\param cm The CManager in use.
\param rpc_name The RPC being called.
\param contact_info An attr_list containing CM-style contact information for the RPC server being called.
\param in_data The input data for the RPC.
\param out_data The output data for the RPC.
\param flags Flags field for the RPC.  If CMrpc_async_call is set, the call is made asynchronously; otherwise, the call blocks until the results arrive.
\return If CMrpc_async_call is set, the return value is a CMrpc_ticket that can be used to retrieve the results of the operation; otherwise, the return value is NULL.
  */
extern CMrpc_ticket
CMrpc_call_rpc ARGS((CManager cm,
                      const char* rpc_name,
                      attr_list contact_info,
                      void* in_data,
                      void* out_data,
                      unsigned long flags));

  /*! Client-side RPC where no reply is expected.  Otherwise functions similarly to CMrpc_call_rpc().
\param cm The CManager in use.
\param rpc_name The RPC being called.
\param contact_info An attr_list containing CM-style contact information for the RPC server being called.
\param in_data The input data for the RPC.
\param flags Flags field for the RPC.  If CMrpc_async_call is set, the call is made asynchronously; otherwise, the call blocks until the results arrive.
  */
extern void
CMrpc_call_rpc_oneway ARGS((CManager cm,
                             const char* rpc_name,
                             attr_list contact_info,
                             void* in_data,
                             unsigned long flags));

  /*! Client-side "anonymous" RPC call. An anonymous call is a call to an RPC that is not registered; applications that will only make a single call to RPCs or call them only in rare cases may choose to use this interface. Note that callers must obtain their own IOContext and IOFormat values from either the CM or PBIO format registration interfaces. Operates in the same manner as CMrpc_call_rpc.
\param cm The CManager in use.
\param rpc_name The RPC for which this format is to be registered.
\param target_spec An attr_list containing CM contact information for the server to be contacted.
\param anon_in_context An IOContext containing all of the format information necessary to encode the parameters to the RPC.
\param in_struct An FMStructDescList describing the RPC input data.
\param anon_out_context An IOContext containing all of the format information necessary to encode the parameters to the RPC.
\param out_struct An FMStructDescList describing the RPC output data.
\param flags Flags field for the RPC.  If CMrpc_async_call is set, the call is made asynchronously; otherwise, the call blocks until the results arrive.
\return If CMrpc_async_call is set, the return value is a CMrpc_ticket that can be used to retrieve the results of the operation; otherwise, the return value is NULL.
\sa CM (CMFormatList), FFS (FMStructDescList) documentation.
  */
extern CMrpc_ticket
CMrpc_call_anon_rpc ARGS((CManager cm,
			  const char* rpc_name,
			  attr_list target_spec,
			  FMStructDescList in_struct,
			  void* in_data,
			  FMStructDescList out_struct,
			  void* out_data,
			  unsigned long flags));

  /*! Asynchronous RPC ticket redemption. A valid CMrpc_ticket can be exchanged for operation results using this function. wait specifies whether the caller wishes to block until results are available or return immediately whether they are available or not. A NULL result indicates that the result was available and has been placed in out_data; otherwise, ticket is returned.
\param cm The CManager in use.
\param ticket The CMrpc_ticket to be redeemed.
\param out_data Output data for the ticket.
\param wait If wait == 1, and the results of the RPC are not yet available, this call blocks until the results arrive.  If wait == 0 and the results are not yet available, this call does not block and sets CMrpc_errno to EAGAIN.  If the results are available, they are decoded and placed into out_data.
\return Returns NULL if the requested results were placed into out_data, the provided ticket if not.
  */
extern int
CMrpc_redeem_ticket ARGS((CManager cm,
			  CMrpc_ticket ticket,
			  void* out_data,
			  int wait));

  /*@}*/



/*
 *
 *  $Log: not supported by cvs2svn $
 *  Revision 1.25  2006/10/11 02:00:19  eisen
 *  Changes so that we don't have cm.h or evpath.h in cmrpc.h.
 *
 *  Revision 1.24  2006/10/10 23:33:03  eisen
 *  Changes so that we don't have cm.h or evpath.h in cmrpc.h.
 *
 *  Revision 1.23  2006/10/09 21:52:30  pmw
 *  changes to run off evpath rather than old-style CM
 *
 *  Revision 1.22  2004/10/14 21:59:19  pmw
 *  Rearranging documentation
 *
 *  Revision 1.21  2004/10/14 21:16:15  pmw
 *  Added doxygen documentation.
 *  Also changed prefix of everything from CM_RPC to CMrpc.  It looks better.
 *
 *  Revision 1.20  2003/10/07 19:49:35  eisen
 *  Add 'extern "C" {' stuff to cmrpc.h header.
 *
 *  Revision 1.19  2002/06/16 21:59:16  pmw
 *  more and more fixes.  This should at least shut up the messages from
 *  the nightly chaos builds.
 *
 *  Revision 1.18  2002/05/27 07:44:55  pmw
 *  const correctness for C compilers sucks.  Especially when I use so much code from
 *  other people that isn't const correct :)
 *
 *  Revision 1.17  2000/10/27 19:28:54  pmw
 *  Removed CM_RPC_register_rpc_subformat().  I didn't understand how CM
 *  worked well enough; this function is really not necessary.
 *
 *  Revision 1.16  2000/10/23 20:52:09  pmw
 *  Added contact_info parameter to call_rpc calls.  This is to be used
 *  if, for a specific invocation of an RPC, you wish to override the
 *  registered contact information.
 *
 *  Revision 1.15  2000/10/16 03:45:35  pmw
 *  Added oneway RPC calls.  This has been tested somewhat.
 *
 *  Revision 1.14  2000/10/16 02:31:21  pmw
 *  1 - fixed gotcha-in-waiting concerning IOcontext management and anonymous
 *  calls.  The IOcontext allocated for anon calls was getting freed before the
 *  user ever saw the result structure from the call, and the underlying PBIO
 *  buffer was suspect.  This meant revising the anon-call interface back to
 *  where the user supplies his own IOcontext values.
 *  2 - Added rpc-with-channel functionality, not yet tested.
 *
 *  Revision 1.13  2000/09/29 20:52:07  pmw
 *  fixed lingering problems from subcontext changes.  hopefully this closes it.
 *  make check works correctly, so I'll believe it for the moment.
 *
 *  Revision 1.12  2000/09/19 20:54:18  pmw
 *  changed syntax of handlers to eliminate explicit passing of CManager value - now contained in options structure
 *
 *  Revision 1.11  2000/05/12 20:54:17  pmw
 *  Re-did anonymous call syntax to avoid PBIO memory-handling issues
 *
 *  Revision 1.10  2000/05/10 19:49:11  pmw
 *  removed extra free() which caused async test bug
 *
 *  Revision 1.9  2000/04/11 18:40:33  pmw
 *  added CM_RPC_errno
 *
 *  Revision 1.8  2000/04/10 21:22:17  pmw
 *  added options flag to handlers
 *
 *  Revision 1.7  2000/04/10 20:28:32  pmw
 *  Added cleanup handler functionality
 *
 *  Revision 1.6  2000/04/10 18:10:14  pmw
 *  more rearrangement
 *
 *  Revision 1.5  2000/04/06 22:39:18  pmw
 *  Now does sane handling of IO conversions
 *
 *  Revision 1.4  2000/04/05 03:00:05  pmw
 *  added high-level convenience functions
 *
 *  Revision 1.3  2000/04/04 20:40:04  pmw
 *  Restructured for asyncronous calls
 *
 *  Revision 1.2  2000/04/04 18:35:29  pmw
 *  added RCS headers
 *
 *
 */
#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* ifndef __CMRPC_H__ */
