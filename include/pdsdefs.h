#ifndef _PDSDEFS_H_
#define _PDSDEFS_H_

/*
 *
 *  $Id: pdsdefs.h,v 1.7 2003-03-19 03:49:27 pmw Exp $
 *
 */
/** @addtogroup pdsgroup
 *
 * @{
 */
#define PDS_ID_SIZE 32

#include "atl.h"

/*! Opaque structure identifying a remote PDS service instance. */
typedef struct _pds_service_struct* pds_service;

/*! ID for PDS entities */
typedef struct _pds_entity_id_t_struct
{
  unsigned char id[PDS_ID_SIZE];
} pds_entity_id_t, *pds_entity_id_t_ptr;

/*! ID for PDS contexts */
typedef struct _pds_context_id_t_struct
{
  unsigned char id[PDS_ID_SIZE];
} pds_context_id_t, *pds_context_id_t_ptr;

/*! ID for PDS domains */
typedef struct _pds_domain_id_t_struct
{
  unsigned char id[PDS_ID_SIZE];
} pds_domain_id_t, *pds_domain_id_t_ptr;

/*! Typedef for entity data stored in PDS.  The structure contains a pointer to the data buffer, 
 *  the size of the data buffer, and an attr_value indicating the type of the data. 
 */
typedef struct _pds_entity_data_t_struct
{
  unsigned char *data;
  int data_size;
  attr_value_type data_type;
} pds_entity_data_t, *pds_entity_data_t_ptr;

/*
 *
 *  Entity/context change event definition
 *
 */
#define PDS_ENTITY_CHANGE_BASE_VAL          300

/*! UID for the ENTITY_CHANGE_CREATION event.  Submitted to the entity's update channel
 *  when the entity is created.  Note that the semantics of update subscription in PDS 
 *  allow interest to be registered for entities that don't exist at subscription time.
 */
#define PDS_ENTITY_CHANGE_CREATION          (PDS_ENTITY_CHANGE_BASE_VAL + 1)

/*! UID for the ENTITY_CHANGE_ATTRS event.  Submitted to the entity's update channel
 *  when the attributes of the entity are updated.
 */
#define PDS_ENTITY_CHANGE_ATTRS             (PDS_ENTITY_CHANGE_BASE_VAL + 2)


/*! UIDs for the ENTITY_CHANGE_BIND/UNBIND events.  Submitted to the entity's update channel 
 *  when the entity is (un)bound (from)to a particular name in a PDS domain.
 */
#define PDS_ENTITY_CHANGE_BIND              (PDS_ENTITY_CHANGE_BASE_VAL + 3)
#define PDS_ENTITY_CHANGE_UNBIND            (PDS_ENTITY_CHANGE_BASE_VAL + 4)


/*! UID for the ENTITY_CHANGE_DELETION event.  Submitted to the entity's update channel 
 *  when the entity is deleted from its hosting PDS service instance.
 */
#define PDS_ENTITY_CHANGE_DELETION          (PDS_ENTITY_CHANGE_BASE_VAL + 5)

/*! UID for the ENTITY_CHANGE_DATA event.  Submitted to the entity's update channel
 *  when the entity's data is modified.
 */
#define PDS_ENTITY_CHANGE_DATA              (PDS_ENTITY_CHANGE_BASE_VAL + 6)

#define PDS_CONTEXT_CHANGE_BASE_VAL         320
#define PDS_CONTEXT_CHANGE_CREATION         (PDS_CONTEXT_CHANGE_BASE_VAL + 1)
#define PDS_CONTEXT_CHANGE_ATTRS            (PDS_CONTEXT_CHANGE_BASE_VAL + 2)
#define PDS_CONTEXT_CHANGE_BIND             (PDS_CONTEXT_CHANGE_BASE_VAL + 3)
#define PDS_CONTEXT_CHANGE_UNBIND           (PDS_CONTEXT_CHANGE_BASE_VAL + 4)
#define PDS_CONTEXT_CHANGE_DELETION         (PDS_CONTEXT_CHANGE_BASE_VAL + 5)

#define PDS_DOMAIN_CHANGE_BASE_VAL	    350
#define PDS_DOMAIN_CHANGE_CONTEXT_CHANGE    (PDS_DOMAIN_CHANGE_BASE_VAL + 1)
#define PDS_DOMAIN_CHANGE_ENTITY_CHANGE     (PDS_DOMAIN_CHANGE_BASE_VAL + 2)
typedef struct _pds_entity_change_event_struct
{
  int type;
  const char *desc;
  const char *fullname;
  pds_entity_id_t entity_id;
  pds_entity_data_t entity_data;
} pds_entity_change_event, *pds_entity_change_event_ptr;

typedef struct _pds_context_change_event_struct
{
  int type;
  char *desc;
  char *fullname;
  pds_context_id_t context_id;
} pds_context_change_event, *pds_context_change_event_ptr;

typedef struct _pds_domain_change_event_struct
{
  int type;
  char* desc;
  char* d_name;
  char* d_type;
  int d_version;
  char* d_app_name;
  pds_domain_id_t domain_id;
  int   event_type;
  char* event_fullname;
  char* event_desc;
} pds_domain_change_event, *pds_domain_change_event_ptr;

/*
 *  bit mask values for options masks
 */
#define OnlyEntityBindings 0x1
#define OnlyContextBindings 0x2

#define GetDataBuffer 0x2
#define GetDataSize 0x4
#define GetDataType 0x8

/** @} */

/*
 *
 *  $Log: not supported by cvs2svn $
 *  Revision 1.6  2003/02/04 17:25:49  mansour
 *  Added defs for domain event types
 *
 *  Revision 1.5  2003/01/30 19:15:42  mansour
 *  Add support for domain events
 *
 *  Revision 1.4  2002/11/16 04:10:22  pmw
 *  added functionality to selectively retrieve entity data/type/size information
 *  using the get_entity_data calls.  New options masks have been added to pdsdefs.h
 *  to make this work.
 *
 *  Revision 1.3  2002/10/30 04:42:00  pmw
 *  typos...
 *
 *  Revision 1.2  2002/10/16 04:58:37  pmw
 *  added initial Doxygen comments
 *
 *  Revision 1.1  2001/10/09 21:37:53  pmw
 *  configuration changes
 *
 *  Revision 1.5  2001/10/07 22:19:16  pmw
 *  Added domain_change event structures so that Proactive class is always correctly
 *  initialized.
 *
 *  Revision 1.4  2001/09/10 03:55:43  pmw
 *  client lib is now libpdsagent, and is compiled entirely with gcc.  libpdsutil no
 *  longer exists.
 *
 *  Revision 1.3  2001/07/02 17:55:22  pmw
 *  hopefully final changes to make contexts emit bind/unbind events
 *
 *  Revision 1.2  2001/07/02 04:02:03  pmw
 *  interim checkin for changes to make Contexts also support events.  compiles
 *  but tests probably do not run.
 *
 *  Revision 1.1.1.1  2000/12/13 22:14:20  pmw
 *  imported sources
 *
 *  Revision 1.10  2000/11/30 01:47:56  pmw
 *  rearranged include files to fix inappropriate inclusions
 *
 *  Revision 1.9  2000/11/22 21:42:37  pmw
 *  proactivity based on entity events now appears to work correctly.  In particular,
 *  creation events based on the prior registration of placeholder entities and their
 *  later "creation" as real entities works.
 *
 *  Revision 1.8  2000/11/19 23:28:31  pmw
 *  wp_server now supports bulk load of data with -f command-line option.  Also added
 *  command message so bulk loads can happen on-line.
 *
 *  Revision 1.7  2000/10/25 21:44:49  pmw
 *  Compiles after major revisions to support multiple WP services on
 *  client-side.  This should eventually allow a client to use services by
 *  linking in the library directly or by communicating using CMRPC.
 *
 *  Revision 1.6  2000/05/15 21:15:48  pmw
 *  minor changes
 *
 *  Revision 1.5  2000/01/06 04:59:07  pmw
 *  changed id scheme so java side is simpler
 *
 *  Revision 1.4  1999/12/21 18:47:57  pmw
 *  Too many changes - really shouldn't have done this many
 *
 *
 */

#endif  /* matches ifndef PDSDEFS_H */
