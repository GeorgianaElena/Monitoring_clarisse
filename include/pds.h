#ifndef _PDS_H_
#define _PDS_H_

/*
 *
 *  $Id: pds.h,v 1.19 2008-09-23 23:57:22 eisen Exp $
 *
 */
/** @defgroup pdsgroup The PDS Agent API
 *  
 * @{
 */


#ifdef __cplusplus
extern "C" {
#endif

#include <sys/types.h>

#include "atl.h"

#include "evpath.h"

#include "pdsdefs.h"


/*! A pre-defined NULL entity ID.  Conceptually equivalent to a NULL char*.
 */
extern pds_entity_id_t null_pds_entity_id;
/*!  A pre-defined NULL context ID.  Conceptually equivalent to a NULL char*.
 */
extern pds_context_id_t null_pds_context_id;
/*!  A pre-defined NULL domain ID.  Conceptually equivalent to a NULL char*.
 */
extern pds_domain_id_t null_pds_domain_id;
  

/*
 * Server API
 */

/*! Open a connection to a PDS service instance.  The service may be local or remote as
 *  specified by the attribute list given as a parameter.  
 *  \param contact_attrs The attribute list used by PDS to contact the PDS service instance through CM.
 *  \return A pointer to a pds_service structure identifying the remote service instance.  Returns NULL if the connection cannot be opened.
 */
pds_service pds_service_open (attr_list contact_attrs);

/*! Close the connection to the given PDS service instance.  
 *  \param service_to_close The local "handle" to the service instance to be closed.  If NULL, this function has no effect.
 *  \return Returns 1 if successful, -1 otherwise.
 */
int pds_service_close (pds_service service_to_close);

/*! Request that the PDS service instance identified by the parameter shut down.  Currently, the service instance is not required to honor the request.
 *  \param pds_service The local "handle" to the service instance to be shut down.  If NULL, this function has no effect.
 *  \return Returns 1 if the request is successfully made, -1 otherwise.  Note that this return value does not indicate whether or not the service instance responded to the request.
 */
int pds_shutdown_server (pds_service);

/*
 * High level API
 */

/*! Look up a string value.
 *  \param domain_id Identifies the domain of the requested value.  
 *  \param fullname This is the context name ("key") for the requested string value.  
 *  \param value A pointer-to-pointer-to-char in which to store the resulting value.  This function allocates memory for this string which must be freed by the caller using free().  If the value is not present or is not an Attr_String value, this parameter is set to NULL.
 *  \return Returns 1 if successful (an Attr_String value was found using the specified name and successfully returned) and -1 otherwise.
 */
int
pds_lookup_string (pds_domain_id_t domain_id,
                  const char* fullname,
                  char **value);

/*! Bind a string to a name in the given domain.  Binding creates an association between the context name and the value which can be queried at a later time.  
 *  \param domain_id Identifies the domain in which to create the binding.
 *  \param fullname Context name to use in the binding.
 *  \param value String to bind.  If value is NULL, a binding to a NULL string is created.
 *  \return Returns 1 if successful, -1 otherwise.
 */
int
pds_bind_string (pds_domain_id_t domain_id,
                const char* fullname,
                const char* value);

/*! Look up an integer (Attr_Int4) value.
 *  \param domain_id Identifies the domain of the requested value.  
 *  \param fullname This is the context name ("key") for the requested value.  
 *  \param value A pointer-to-int in which to store the resulting value.  If the value is not present or is not an Attr_Int4 value, this parameter is set to NULL.
 *  \return Returns 1 if successful (an Attr_Int4 value was found using the specified name and successfully returned) and -1 otherwise.
 */
int
pds_lookup_int (pds_domain_id_t domain_id,
               const char* fullname,
               int* value);

/*! Bind an integer to a name in the given domain.  Binding creates an association between the context name and the value which can be queried at a later time.  
 *  \param domain_id Identifies the domain in which to create the binding.
 *  \param fullname Context name to use in the binding.
 *  \param value Integer value to bind.  
 *  \return Returns 1 if successful, -1 otherwise.
 */
int
pds_bind_int (pds_domain_id_t domain_id,
             const char* fullname,
             int value);

/*! Look up a blob (binary large object - Attr_Unknown) value.
 *  \param domain_id Identifies the domain of the requested value.  
 *  \param fullname This is the context name ("key") for the requested value.  
 *  \param value A pointer-to-pointer-to-char in which to store the resulting value.  This function allocates memory for this string which must be freed by the caller using free().  If the value is not present or is not an Attr_Unknown value, this parameter is set to NULL.
 *  \return Returns 1 if successful (an Attr_Unknown value was found using the specified name and successfully returned) and -1 otherwise.
 */
int
pds_lookup_blob (pds_domain_id_t domain_id,
                const char* fullname,
                char** value);

/*! Bind a blob (binary large object) to a name in the given domain.  Binding creates an association between the context name and the value which can be queried at a later time.  
 *  \param domain_id Identifies the domain in which to create the binding.
 *  \param fullname Context name to use in the binding.
 *  \param value Byte buffer containing object to bind.
 *  \param length Length of the byte buffer.
 *  \return Returns 1 if successful, -1 otherwise.
 */
int
pds_bind_blob (pds_domain_id_t domain_id,
              const char* fullname,
              char *value,
              unsigned int length);

/*! Load the directory with a content file found at the given URL.  Testing or initialization purposes may require that a service instance contain a known set of information.  This function retrieves an XML file containing one or more PDS bulk-load sets and adds its contents to the in-memory state of the instance.  The format of the XML file is defined elsewhere (yeah right).  The end state of the service instance is the union of any content existing when this function is called and the content of the file.  Common context values between pre-existing content and the file are bound to entity values found in the file.  Existing event registrations are not affected, and all defined change events are fired.  The bulk-load may cause entity placeholders to be promoted to live entities; i.e. there is no way currently to register interest in an entity using the bulk-load process without explicitly creating it (implying that no creation events are fired by the bulk-load).

Currently the service instance is not required to honor the request to perform the bulk-load.
 *  \param s Handle for the service instance.
 *  \param url Universal Resource Locator specifying the location of the XML file containing the desired bulk-load set(s).
 *  \return Returns 1 if the request is completed successfully.
 */
int
pds_bulk_load_from_URL (pds_service s,
                       const char* url);

/*
 * Domain API
 */
/*! Create a new domain in the given service instance.  If a domain matching the given information already exists, its domain ID is returned instead.
 * \deprecated This function will go away.  Use \p pds_open_domain instead.
 * \param s The handle to the service instance.
 * \param domain_name Name of the new domain.
 * \param domain_type Type of the new domain.
 * \param domain_version Version number.
 * \param application_name Self-explanatory.
 * \return Returns the new domain ID.  If the operation is unsuccessful, a null domain ID (equivalent to \p null_pds_domain_id) is returned instead.
 */
pds_domain_id_t
pds_create_domain (pds_service s,
                  const char *domain_name,
		  const char *domain_type,
		  int domain_version,
		  const char *application_name);

/*! Open an existing domain in the given service instance.  If no domain matching the given parameters is found, a new domain is created and its ID returned.
 * \deprecated This function will go away.  Use \p pds_open_domain instead.
 * \param s The handle to the service instance.
 * \param domain_name Name of the new domain.
 * \param domain_type Type of the new domain.
 * \param domain_version Version number.
 * \param application_name Self-explanatory.
 * \return Returns the opened domain ID.  If the operation is unsuccessful, a null domain ID (equivalent to \p null_pds_domain_id) is returned instead.
 */
pds_domain_id_t
pds_open_domain (pds_service s,
		const char *domain_name,
		const char *domain_type,
		int domain_version,
		const char *application_name);

/*! Remove the indicated domain from its service instance.  The handle to the instance is not necessary since domains know what service instance they belong to.  All events for context destruction and entity unbinding/destruction are fired.
 *  \param id The domain to remove.
 *  \return Returns 1 if the operation is successful, 0 otherwise.
 */
int
pds_remove_domain (pds_domain_id_t id);

/*! Locate domains matching the given characteristics.  Limited string-matching is performed; i.e. wildcards are not generally supported.  If you do not want to search based on one of the items, do not specify a value for that item (pass NULL for \c domain_type, for example).  All domains matching the given criteria are returned in the NULL-terminated list \c matches.  The caller is responsible for calling \c free() on this list when finished using it.
 * \param s The PDS service instance to search in.
 * \param domain_name Name criteria.
 * \param domain_type Type criteria.
 * \param domain_version Version number.
 * \param application_name Self-explanatory.
 * \param matches NULL-terminated list of pds_domain_id_t containing the matches found by the search.  Pass the address of a pds_domain_id_t*.  Will be set to NULL if there are no matches.
 * \return Returns 1 if the operation is successful, -1 if there was a problem.  Note that a search returning no domain IDs is still considered a successful operation - check the matches parameter for results of the search.
 */
int
pds_find_matching_domains (pds_service s,
                          const char *domain_name,
			  const char *domain_type,
			  int domain_version,
			  const char *application_name,
                          pds_domain_id_t **matches);

  /* @{ */
  /*! Set/get the attributes of a domain.  The given attribute list replaces the previous attribute list. 
   *  \param d_id The domain ID desired.
   *  \param al The new attribute list to associate with the domain.
   *  \return Returns 1 if the operation was successful, -1 otherwise.
   */
int
pds_set_domain_attrs (pds_domain_id_t d_id, attr_list al);

int
pds_get_domain_attrs (pds_domain_id_t d_id, attr_list al);
  /* @} */


/*
 *  Context API
 */

/*!
  Create a context in the given domain.  The \a name and \a c_id parameters are used as follows.  String-based naming for contexts and entities in PDS is the preferred method, and results in names like "/path/to/the/entity" being used to identify these objects.  In such a name string, "/", "/path", "/path/to", and "/path/to/the" all identify contexts.  Each of these contexts has associated with it a context ID.  Relative naming applies a "base" context ID to a string-based name.  For example, if \c my_context_ID is the pds_context_id_t corresponding to the context named by "/path/to", then absolute naming using "/path/to/the/entity" and relative naming that applies the string "/the/entity" to \c my_context_ID both refer to the same object.  
  PDS context functions make use of this property by allowing the caller to specify a context ID.  If it is present, relative naming is assumed and the \a name parameter is used as a string relative to the given context ID.  If the context ID is \c null_pds_context_id, then absolute naming is assumed.
  \param d_id The domain in which to create the new context.
  \param name The "trailing component" of the full context name.  Must not be NULL; i.e. must contain at least one name component. 
  \param c_id The context ID used as a "base context" for the trailing component.  Pass \c null_pds_context_id to indicate that the \a name parameter contains the entire name for the new context.
  \return Returns a \c pds_context_id_t identifying the newly created context.
*/
pds_context_id_t
pds_create_context (pds_domain_id_t d_id, const char *name, pds_context_id_t c_id);

  /*!
    Remove a context from the given domain.  All bindings held by the context are unbound and associated context/entity unbinding events fired.  See \c pds_create_context for a discussion of context naming and the use of \a name and \a c_id.
  \param d_id The domain from which to remove the indicated context.
  \param name The "trailing component" of the full context name.  Must not be NULL; i.e. must contain at least one name component. 
  \param c_id The context ID used as a "base context" for the trailing component.  Pass \c null_pds_context_id to indicate that the \a name parameter contains the entire name for the context.
  \return Returns 1 if the operation is successful, -1 otherwise.
  */
int
pds_remove_context (pds_domain_id_t d_id, const char *name, pds_context_id_t c_id);

  /*!
    Remove a context from the given domain, specifying it directly by its ID.  All bindings held by the context are unbound and associated context/entity unbinding events fired.
    \param d_id The domain from which to remove the indicated context.
    \param c_id The context to remove.
    \return Returns 1 if the operation is successful, -1 otherwise.
  */
int
pds_remove_context_by_id (pds_domain_id_t d_id, pds_context_id_t c_id);

  /*!
    Bind a context to another context in the tree.  This function expects that the source context is already created to a name in the domain.
    \param d_id The domain where the entity should be bound.
    \param source_context The source context we plan to create a link for.
    \param link_name the full path of the new linked context
    \return Returns 1 if the new binding is successful, -1 otherwise.
  */
int
pds_bind_context_by_id (pds_domain_id_t d_id,                        
                        pds_context_id_t source_context,
                        const char *link_name);

  /*!
    Bind a context to another context in the tree.  This function expects that the source context is already created to a name in the domain.
    \param d_id The domain where the entity should be bound.
    \param source_name The source context we plan to create a link for.
    \param link_name the full path of the new linked context
    \return Returns 1 if the new binding is successful, -1 otherwise.
  */
int
pds_bind_context_by_name (pds_domain_id_t d_id,                        
                        const char* source_name,
                        const char *link_name);

  /*!
    Unbind a context from a name.  \a name is used to identify a context binding, and then that binding is removed.
    \param d_id The domain where the context-name binding exists.
    \param name The identifying name of the context binding. Must not be NULL; i.e. must contain at least one name component.
    \return Returns 1 if the operation is successful, -1 otherwise.
  */
int
pds_unbind_context_from_name (pds_domain_id_t d_id, const char *name);

  /*!
    Get the context ID of the root context of the domain's namespace.
    \param d_id The domain ID of the domain in question.
    \return Returns the pds_context_id_t of the domain's root context.
  */
pds_context_id_t
pds_get_root_context (pds_domain_id_t d_id);

  /*!
    Get the binding list of a context.  The binding list consists of all names bound to either contexts or entities in that context.  This function allocates memory for its return value which the caller must release after use by calling \c free().
    \param d_id The domain ID of the context.
    \param name The absolute name of the context in question.
    \param opt_mask Options mask indicating variations.  Pass \c OnlyEntityBindings to return only names bound to entities in the given context.  Pass \c OnlyContextBindings to return only names bound to contexts.  Pass 0 to return all bindings.
    \return Returns a NULL-terminated array of strings as the binding list.
  */
char **
pds_get_binding_list (pds_domain_id_t d_id, const char* name, int opt_mask);

  /*!
    Get the binding list of a context.  The binding list consists of all names bound to either contexts or entities in that context.  This function allocates memory for its return value which the caller must release after use by calling \c free().
    \param d_id The domain ID of the context.
    \param c_id The context ID of the context in question.
    \param opt_mask Options mask indicating variations.  Pass \c OnlyEntityBindings to return only names bound to entities in the given context.  Pass \c OnlyContextBindings to return only names bound to contexts.  Pass 0 to return all bindings.
    \return Returns a NULL-terminated array of strings as the binding list.
  */
char**
pds_get_binding_list_id (pds_domain_id_t d_id, pds_context_id_t c_id, int opt_mask);



/*
 *  Entity API
 */

  /*!
    Create an entity in the given context in the given domain.  Relative/absolute naming applies with the \a fullname and \a c_id parameters as described in the context API.  
    \param d_id The domain ID in which to create the entity.
    \param fullname The "trailing component" of the full context name.  Must not be NULL; i.e. must contain at least one name component. 
    \param c_id The context on which to base the \a fullname argument.
    \param edata A pointer to a \c pds_entity_data_t structure.  Pass NULL if no entity data is to be stored.
    \param attribute_list An \c attr_list to be associated with the entity.  Pass NULL if no attributes are to be stored.
    \return Returns the ID of the newly created entity, or \c null_pds_entity_id if the operation is unsuccessful.
  */
pds_entity_id_t
pds_create_entity (pds_domain_id_t d_id,
                  const char *fullname,
                  pds_context_id_t c_id,
                  pds_entity_data_t *edata,
                  attr_list attribute_list);

  /*!
    Remove an entity from the given domain.  Relative/absolute naming applies with the \a fullname and \a c_id parameters as described in the context API.  
    \param d_id The domain ID from which to delete the entity.
    \param fullname The "trailing component" of the full context name.  Must not be NULL; i.e. must contain at least one name component. 
    \param c_id The context on which to base the \a fullname argument.
    \return Returns 1 if the deletion is successful, -1 otherwise. 
  */
int
pds_remove_entity (pds_domain_id_t d_id, const char *fullname, pds_context_id_t c_id);

  /*!
    Remove an entity from the given Domain, identifying the entity by its ID.
    \param d_id The domain from which to delete the entity.
    \param e_id The ID of the entity to delete.  
    \return Returns 1 if the deletion is successful, -1 otherwise.
  */
int
pds_remove_entity_by_id (pds_domain_id_t d_id, pds_entity_id_t e_id);

  /*!
    Bind an entity to a name in a specific context.  This function expects that the entity is already bound to a name in the domain and so can be identified by that name (relative to a context ID if desired).  Thus this function serves to create alternate bindings to a single entity.
    \param d_id The domain where the entity should be bound.
    \param source_name The "trailing component" of the name at which the entity can be found.  Must not be NULL; i.e. must contain at least one name component.
    \param source_context The context on which to base the \a source_name.
    \param new_name The "trailing component" of the name to bind the entity.  Must not be NULL; i.e. must contain at least one name component.
    \param new_name_context The context on which to base the \a new_name.
    \return Returns 1 if the new binding is successful, -1 otherwise.
  */
int
pds_bind_entity_to_name (pds_domain_id_t d_id,
			const char *source_name,
                        pds_context_id_t source_context,
			const char *new_name,
                        pds_context_id_t new_name_context);

  /*!
    Bind an entity to a name in a specific context.  This function expects that the entity is identified by its ID.
    \param d_id The domain where the entity should be bound.
    \param e_id The ID of the entity to bind.
    \param new_name The "trailing component" of the name to bind the entity.  Must not be NULL; i.e. must contain at least one name component.
    \param new_name_context The context on which to base the \a new_name.  Pass pds_null_context_id if \a name is an absolute name.
    \return Returns 1 if the new binding is successful, -1 otherwise.
  */
int
pds_bind_entity_to_name_by_id (pds_domain_id_t d_id, 
                              pds_entity_id_t e_id, 
                              const char *new_name,
                              pds_context_id_t new_name_context);

  /*!
    Unbind an entity from a name.  \a name and \a c_id are used to identify an entity, and then that binding to the entity is removed.
    \param d_id The domain where the entity-name binding exists.
    \param name The "trailing component" of the name identifying the entity.  Must not be NULL; i.e. must contain at least one name component.
    \param c_id The context on which to base \a name.  Pass pds_null_context_id if \a name is an absolute name.
    \return Returns 1 if the operation is successful, -1 otherwise.
  */
int
pds_unbind_entity_from_name (pds_domain_id_t d_id, const char *name, pds_context_id_t c_id);

  /*!
    Obtain an entity ID from a name.  
    \param d_id The domain where the entity-name binding exists.
    \param name The "trailing component" of the name identifying the entity.  Must not be NULL; i.e. must contain at least one name component.
    \param c_id The context on which to base \a name.  Pass pds_null_context_id if \a name is an absolute name.
    \return Returns 1 if the operation is successful, -1 otherwise.
  */
pds_entity_id_t
pds_resolve_entity_name (pds_domain_id_t d_id, const char *name, pds_context_id_t c_id);

  /*!
    Register for any changes on the given domain.  The handler function specified by \a func_ptr is called with domain update events.  
    \param cm A \c CManager value.  This can be obtained using the \c CManager_create() function from CM.
    \param d_id The ID of the domain about which to register for changes.
    \param func_ptr A pointer to a handler function for change events.
    \param client_data A pointer to a data structure that will be passed to the event handler each time an update occurs.  For more information, see the ECho documentation.
    \return Returns an \c ECSinkHandle from ECho.  If unsuccessful, returns NULL.
  */
EVaction
pds_register_for_domain_changes (CManager cm,
                                pds_domain_id_t d_id,
                                EVSimpleHandlerFunc func_ptr,
                                void *client_data);

  /*!
    Register for any changes on the given entity.  The handler function specified by \a func_ptr is called with entity update events.  
    \param cm A \c CManager value.  This can be obtained using the \c CManager_create() function from CM.
    \param d_id The ID of the domain where the entity resides.
    \param name The "trailing component" of the name identifying the entity.  Must not be NULL; i.e. must contain at least one name component.
    \param c_id The context on which to base \a name.  Pass pds_null_context_id if \a name is an absolute name.
    \param func_ptr A pointer to a handler function for change events.
    \param client_data A pointer to a data structure that will be passed to the event handler each time an update occurs.  For more information, see the ECho documentation.
    \return Returns an \c ECSinkHandle from ECho.  If unsuccessful, returns NULL.
  */
EVaction
pds_register_for_entity_changes (CManager cm,
                                pds_domain_id_t d_id,
				const char *name,
				pds_context_id_t c_id,
				EVSimpleHandlerFunc func_ptr,
				void *client_data);

  /*!
    Register for any changes on the given context.  The handler function specified by \a func_ptr is called with entity update events.  
    \param cm A \c CManager value.  This can be obtained using the \c pds_get_CManager() function.
    \param d_id The ID of the domain where the context resides.
    \param name The "trailing component" of the name identifying the context.  Must not be NULL; i.e. must contain at least one name component.
    \param c_id The context on which to base \a name.  Pass pds_null_context_id if \a name is an absolute name.
    \param func_ptr A pointer to a handler function for change events.
    \param client_data A pointer to a data structure that will be passed to the event handler each time an update occurs.  For more information, see the ECho documentation.
    \return Returns an \c ECSinkHandle from ECho.  If unsuccessful, returns NULL.
  */
EVaction
pds_register_for_context_changes (CManager cm,
                                  pds_domain_id_t d_id,
                                  const char *name,
                                  pds_context_id_t c_id,
                                  EVSimpleHandlerFunc func_ptr,
                                  void *client_data);

  /*!
    Cancel the registration for a particular sink handle.
    \param sink_handle The sink handle (obtained from one of the registration functions) identifying the event registration to cancel.
    \return Returns 1 if the cancellation is successful, -1 otherwise.
  */
int
pds_cancel_changes_registration (EVaction);

  /*!
    Return the entity data for a particular entity.
    \param d_id The domain where the entity-name binding exists.
    \param name The "trailing component" of the name identifying the entity.  Must not be NULL; i.e. must contain at least one name component.
    \param c_id The context on which to base \a name.  Pass pds_null_context_id if \a name is an absolute name.
    \param edata The address of a structure to populate with the entity data.  Memory for the structure's \c data member is allocated automatically and must be released using \c free().  
    \param opt_mask TBA.
    \return Returns 1 if the operation is successful, -1 otherwise.
  */
int
pds_get_entity_data (pds_domain_id_t d_id,
                    const char *name,
                    pds_context_id_t c_id,
                    pds_entity_data_t *edata,
                    int opt_mask);


  /*!
    Return the entity data for a particular entity.  The entity is identified by its entity ID.
    \param d_id The domain where the entity-name binding exists.
    \param e_id The ID of the entity to be modified.
    \param edata The address of a structure to populate with the entity data.  Memory for the structure's \c data member is allocated automatically and must be released using \c free().  
    \param opt_mask TBA.
    \return Returns 1 if the operation is successful, -1 otherwise.
  */
int
pds_get_entity_data_by_id (pds_domain_id_t d_id, pds_entity_id_t e_id, pds_entity_data_t *edata, int opt_mask);


  /*!
    Set the entity data for a particular entity.
    \param d_id The domain where the entity-name binding exists.
    \param name The "trailing component" of the name identifying the entity.  Must not be NULL; i.e. must contain at least one name component.
    \param c_id The context on which to base \a name.  Pass pds_null_context_id if \a name is an absolute name.
    \param edata The address of a structure containing the entity data to store.  
    \param opt_mask TBA.
    \return Returns 1 if the operation is successful, -1 otherwise.
  */
int
pds_set_entity_data (pds_domain_id_t d_id,
                    const char *name,
                    pds_context_id_t c_id,
                    pds_entity_data_t *edata,
                    int opt_mask);
  
  /*!
    Set the entity data for a particular entity.  The entity is identified by its entity ID.
    \param d_id The domain where the entity-name binding exists.
    \param e_id The ID of the entity to be modified.
    \param edata The address of a structure containing the entity data to store.  
    \param opt_mask TBA.
    \return Returns 1 if the operation is successful, -1 otherwise.
  */
int
pds_set_entity_data_by_id (pds_domain_id_t d_id, pds_entity_id_t e_id, pds_entity_data_t *edata, int opt_mask);

  /*!
    Get the entity attributes for a particular entity. 
    \param d_id The domain where the entity-name binding exists.
    \param name The "trailing component" of the name identifying the entity.  Must not be NULL; i.e. must contain at least one name component.
    \param c_id The context on which to base \a name.  Pass pds_null_context_id if \a name is an absolute name.
    \param attributes An \c attr_list to be populated with the entity attributes.  Attributes retrieved from the service are merged with existing attributes.
    \param opt_mask TBA.
    \return Returns 1 if the operation is successful, -1 otherwise.
  */
int
pds_get_entity_attributes (pds_domain_id_t d_id, const char *name, pds_context_id_t c_id, attr_list attrs);

  /*!
    Get the entity attributes for a particular entity.   The entity is identified by its entity ID.
    \param d_id The domain where the entity-name binding exists.
    \param e_id The ID of the entity to be modified.
    \param attributes An \c attr_list to be populated with the entity attributes.  Attributes retrieved from the service are merged with existing attributes.
    \param opt_mask TBA.
    \return Returns 1 if the operation is successful, -1 otherwise.
  */
int
pds_get_entity_attributes_by_id (pds_domain_id_t d_id, pds_entity_id_t e_id, attr_list attributes);

  /*!
    Set the entity attributes for a particular entity. 
    \param d_id The domain where the entity-name binding exists.
    \param name The "trailing component" of the name identifying the entity.  Must not be NULL; i.e. must contain at least one name component.
    \param c_id The context on which to base \a name.  Pass pds_null_context_id if \a name is an absolute name.
    \param attributes The \c attr_list to be stored as the entity attributes.  
    \param opt_mask TBA.
    \return Returns 1 if the operation is successful, -1 otherwise.
  */
int
pds_set_entity_attributes (pds_domain_id_t d_id, const char *name, pds_context_id_t c_id, attr_list attrs, int opt);

  /*!
    Set the entity attributes for a particular entity.  The entity is identified by its entity ID.
    \param d_id The domain where the entity-name binding exists.
    \param e_id The ID of the entity to be modified.
    \param attributes The \c attr_list to be stored as the entity attributes.  
    \param opt_mask TBA.
    \return Returns 1 if the operation is successful, -1 otherwise.
  */
int
pds_set_entity_attributes_by_id (pds_domain_id_t d_id, pds_entity_id_t e_id, attr_list attributes, int opt);

  /*!
    Search the domain for entities matching the attributes contained in \a attributes.  This function can be used to search for entities across the entire domain or only those bound in a given context.  This context is identified in this function by the absolute/relative naming scheme described in the Context API.  If \a name is NULL and \a c_id is null_pds_context_id, then the entire domain is searched.  Subset-equality is used for the match - if the given attribute list is exactly equal or a subset of the attribute list of an entity, the entity is considered a match.  
    \param d_id The domain in which to perform the search.
    \param name The "trailing component" of the name identifying the context to be searched.  Must not be NULL if \a c_id is not \c pds_null_context_id.  
    \param c_id The context on which to base \a name.  Pass pds_null_context_id if \a name is an absolute name.
    \param attributes An \c attr_list to match against attributes contained in potential matching entities.
    \return Returns a \c pds_null_entity_id -terminated list of entity IDs that match the search.  This list is allocated by the function and must be released by the caller using \c free().  If the search returns no entities, the return value is NULL.
  */
pds_entity_id_t*
pds_find_matching_entities (pds_domain_id_t d_id, const char* name, pds_context_id_t c_id, attr_list attributes);

  /*!
    Search the domain for entities matching the attributes contained in \a attributes.  This function can be used to search for entities across the entire domain or only those bound in a given context.  This context is identified in this function by the absolute name \a fullname.  If \a fullname is NULL, then the entire domain is searched.  Subset-equality is used for the match - if the given attribute list is exactly equal or a subset of the attribute list of an entity, the entity is considered a match.  
    \param d_id The domain in which to perform the search.
    \param fullname The name of a context to limit the search.  If this parameter is NULL, the entire domain is searched.
    \param attributes An \c attr_list to match against attributes contained in potential matching entities.
    \return Returns a \c pds_null_entity_id -terminated list of entity IDs that match the search.  This list is allocated by the function and must be released by the caller using \c free().  If the search returns no entities, the return value is NULL.
  */
pds_entity_id_t*
pds_find_matching_entities_by_name (pds_domain_id_t d_id, const char *fullname, attr_list attributes);

   /*!
     A structure used to define criteria for xpath queries.
   */
typedef struct pds_query_struct
{
  char* entity;
  char* op;
  char* value;
} pds_query_struct; 

  /*!
    Constructs an xpath query expression from criteria.
    criteria is an array of pds_query_struct elements, the last element
    must have entity set to zero.
    
    join_operator determines if the array elements are AND'd or OR'd
    
    This function is local on the client side and requires not interaction
    with the server.
    
    \param criteria The criteria expressions
    \return An xpath expression that can be passed to pds_query_by_xpath
            The memory returned must be free'd by the caller.
  */
#define PDS_AND_JOIN 0
#define PDS_OR_JOIN  1
char* 
pds_build_xpath_query(pds_query_struct* criteria, int join_operator);

  /*!
    Constructs an xpath query expression from criteria.
    The expression is a Product of Sums of all criteria.
    Each row in criteria is OR'd together, then the resulting
    expressions are AND'd together.
    This function is local on the client side and requires not interaction
    with the server.
    \param criteria The criteria expressions
    \param num Number of rows in criteria
    \return An xpath expression that can be passed to pds_query_by_xpath
            The memory returned must be free'd by the caller.
  */
char* 
pds_build_extended_xpath_query(pds_query_struct* criteria[], int num);

  /*!
     Matches the xpath expression against the PDS schema
	 and returns the matching elements.
	 The server will return a list of full_name attributes of 
	 the matching elements. The xpath can match contexts or
	 entities.
	 \param xpath An XPath expression
	 \return a list of strings
   */
char**
pds_query_by_xpath(pds_domain_id_t d_id, const char* xpath);


  /*!
    Returns the CManager value used by the PDS client-side library.
    \return Returns a CManager.
  */
CManager
pds_get_CManager();

  /** @} */

#ifdef __cplusplus
} // matches extern "C"
#endif

#endif


/*
 *
 *  $Log: not supported by cvs2svn $
 *  Revision 1.18  2008/06/23 05:10:27  pmw
 *  Removed inclusion of config.h - shouldn't be in any installed file.
 *
 *  Revision 1.17  2007/12/04 21:06:14  pmw
 *  Merge changes from evpath branch.  Also probably includes
 *  Darwin fixes.  Compiles on isleroyale and marquesas, I think
 *  I'm misconfigured somehow for building on gondor.
 *
 *  Revision 1.16.2.1  2007/01/05 22:40:27  pmw
 *  interim commit for evpath changes, new branch
 *
 *  Revision 1.16  2005/12/08 14:39:02  eisen
 *  remove cm.h from include file so it can be evpath.h if the apps need it...
 *
 *  Revision 1.15  2004/11/06 22:23:32  mansour
 *  Added a bind_context_by_name API. Also fixed the XML logic to use the correct bind name.
 *
 *  Revision 1.14  2004/11/03 06:58:11  mansour
 *  Added APIs for context binding. The XML dump routine expands bounded contexts in
 *  to a full tree and the load routine loads it as separate contexts.
 *
 *  Revision 1.13  2003/11/09 03:56:36  mansour
 *  Add wrapper API for building XPath queries
 *
 *  Revision 1.12  2003/10/31 05:38:30  mansour
 *  Added xpath query support
 *
 *  Revision 1.11  2003/10/20 17:06:47  pmw
 *  Changed documentation to reflect the impending demise of pds_create_domain
 *
 *  Revision 1.10  2003/10/05 12:10:23  pmw
 *  Changes to make utils a C module instead of C++, allowing people to
 *  link against its functions more easily
 *
 *  Revision 1.9  2003/03/19 03:49:27  pmw
 *  more documentation tweaks
 *
 *  Revision 1.8  2003/03/18 20:47:32  pmw
 *  added some more documentation stuff
 *
 *  Revision 1.7  2002/10/30 03:52:50  pmw
 *  added parameter to get_binding_list functions to optionally get only entities
 *  or contexts
 *
 *  Revision 1.6  2002/10/22 02:38:12  pmw
 *  Finished Doxygen comments for API.
 *
 *  Revision 1.5  2002/10/21 22:40:21  pmw
 *  Still more comments
 *
 *  Revision 1.4  2002/10/16 21:27:30  pmw
 *  More comments.
 *
 *  Revision 1.3  2002/10/16 04:58:37  pmw
 *  added initial Doxygen comments
 *
 *  Revision 1.2  2002/08/07 18:51:43  eisen
 *  Widespread changes to modify how attribute handling works.  Stop using
 *  attr_atom_from_string() to get atom values.  Instead, those are set by the
 *  application.  Stop using get_attr() (api has changes).  Stop using
 *  xmit_object and start using the ATL-supplied encode/decode routines.
 *
 *  Revision 1.1  2001/10/09 21:37:53  pmw
 *  configuration changes
 *
 *  Revision 1.5  2001/09/23 22:55:54  pmw
 *  Contexts now support attributes of their own.  Practically, this is supported
 *  by having ContextBindable host the attribute list instead of Entity/Context.  Also
 *  added the usual functions on the client side and handlers for getting/setting
 *  this attr list for Contexts.
 *
 *  Revision 1.4  2001/09/10 03:55:43  pmw
 *  client lib is now libpdsagent, and is compiled entirely with gcc.  libpdsutil no
 *  longer exists.
 *
 *  Revision 1.3  2001/09/03 20:09:10  pmw
 *  changes to deal with string-marshaled attr lists
 *
 *  Revision 1.2  2001/07/02 15:07:43  pmw
 *  removed ContextBindable class def and impl to their own source files
 *
 *  Revision 1.1.1.1  2000/12/13 22:14:21  pmw
 *  imported sources
 *
 *  Revision 1.16  2000/12/05 00:24:20  pmw
 *  Added server_shutdown message
 *
 *  Revision 1.15  2000/11/30 01:47:55  pmw
 *  rearranged include files to fix inappropriate inclusions
 *
 *  Revision 1.14  2000/11/29 04:37:37  pmw
 *  changed channel-handling again, Entity now performs most of its own
 *  change event sends
 *
 *  Revision 1.13  2000/11/22 21:42:36  pmw
 *  proactivity based on entity events now appears to work correctly.  In particular,
 *  creation events based on the prior registration of placeholder entities and their
 *  later "creation" as real entities works.
 *
 *  Revision 1.12  2000/11/21 21:35:28  pmw
 *  Domain change events now work correctly.  moved event transmission
 *  responsibilities to Proactive class.
 *
 *  Revision 1.11  2000/11/21 00:23:56  pmw
 *  Since I didn't have enough to do, a little code reorganization to keep
 *  things from getting dramatically out of hand.  Fixed a couple of
 *  bugs-in-waiting.
 *
 *  Revision 1.10  2000/11/20 05:08:53  pmw
 *  added pds_open_domain call.  semantics are such that a domain id is always
 *  returned, either because one matching the criteria was found or a new one
 *  was created because no matches were found.
 *
 *  Revision 1.9  2000/11/19 23:28:31  pmw
 *  pds_server now supports bulk load of data with -f command-line option.  Also added
 *  command message so bulk loads can happen on-line.
 *
 *  Revision 1.8  2000/10/25 21:44:49  pmw
 *  Compiles after major revisions to support multiple WP services on
 *  client-side.  This should eventually allow a client to use services by
 *  linking in the library directly or by communicating using CMRPC.
 *
 *  Revision 1.7  2000/09/19 21:12:12  pmw
 *  revised for new cmrpc handler interface
 *
 *  Revision 1.6  2000/04/10 20:00:13  pmw
 *  completed port to CM from DataExchange
 *
 *  Revision 1.5  2000/01/06 04:59:06  pmw
 *  changed id scheme so java side is simpler
 *
 *  Revision 1.4  1999/12/21 18:47:56  pmw
 *  Too many changes - really shouldn't have done this many
 *
 *  Revision 1.3  1999/12/09 22:35:15  pmw
 *  added client_priv header, RCS comments
 *
 *
 */
