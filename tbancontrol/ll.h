
#ifndef __LL_H
#define __LL_H


/**********************************************************************
 * Name        : MetaNode
 * Description : This is the definition of the MetaNode used by the
 *               linked list implementation. This is probably never
 *               being used since the linked list implementation hides
 *               it in the MetaInformation struct.
 *               data = A pointer to the data for this node
 *               next = A pointer to the next item in the list
 *               prev = A pointer to the previous item in the list
 **********************************************************************/
struct MetaNode {
  void* data;
  void* next;
  void* prev;
};
typedef struct MetaNode MetaNode;


/**********************************************************************
 * Name        : MetaInformation
 * Description : The MetaInformation struct is used to abstract the
 *               usage of the linked list away from the implementation
 *               below. The user can simply accept the structure when
 *               calling the init function and then only pass it as an
 *               argument to the other functions manipulating the
 *               list. There is no need for the user to do anything
 *               manually with this struct.
 **********************************************************************/
struct MetaInformation {
  MetaNode* head;
  MetaNode* tail;
};
typedef struct MetaInformation MetaInformation;


/**********************************************************************
 * Return codes from Linked List functions
 **********************************************************************/
#define LL_OK                     0  /* Command executed with no errors */
#define LL_NO_ACTION              1  /* Command executed but didn't do
                                        anything */
#define LL_FUNCTION_NULL_POINTER  2  /* A NULL pointer was used as a
                                      * callback function, this is
                                      * invalid and no action has been
                                      * performed on the list. */
#define LL_MI_NULL_POINTER        3  /* A NULL pointer was used as the
                                        Meta information parameter to a
                                        function */


MetaInformation* ll_init();
int ll_append(MetaInformation* mi, void* data);
MetaNode* ll_get(MetaInformation* mi, int number);
int ll_removeindex(MetaInformation* mi, int index);


#endif /* __LL_H */
