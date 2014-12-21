
#include "ll.h"
#include <stdlib.h>


/**********************************************************************
 * Name        : ll_init
 * Description : Create the MetaInformation structure.
 * Arguments   : none
 * Returning   : A pointer to the MetaInformation structure.
 * Precond     : -
 * Postcond    : Memory has been allocated
 **********************************************************************/
MetaInformation* ll_init() {
  MetaInformation* mi;

  /* Create the Meta Information struct */
  mi = malloc(sizeof(MetaInformation));

  /* Add the default nodes to it */
  mi->tail = malloc(sizeof(MetaNode));
  mi->head = malloc(sizeof(MetaNode));

  /* Head node */
  mi->head->next = mi->tail;
  mi->head->prev = NULL;

  /* Tail node */
  mi->tail->next = NULL;
  mi->tail->prev = mi->head;

  return mi;
}


/**********************************************************************
 * Name        : ll_prepend
 * Description : Prepend a node to the list
 * Arguments   : mi = The list
 *               data = The data to be added
 * Returning   : LL_OK
 *               LL_MI_NULL_POINTER
 * Precond     : mi must be created
 * Postcond    : mi is changed (a node added)
 **********************************************************************/
int ll_prepend(MetaInformation* mi, void* data) {
  MetaNode* n;

  /* Sanity check */
  if(mi == NULL) {
    return LL_MI_NULL_POINTER;
  }

  /* Create the new node */
  n = malloc(sizeof(MetaNode));
  n->data = data;

  /* Link it to the current first node */
  n->next = mi->head->next;
  n->prev = mi->head;
  ((MetaNode*) n->next)->prev = n;

  /* Put it first in the list */
  mi->head->next = n;

  return LL_OK;
}


/**********************************************************************
 * Name        : ll_append
 * Description :
 * Arguments   :
 * Returning   : LL_OK
 *               LL_MI_NULL_POINTER
 * Precond     :
 * Postcond    :
 **********************************************************************/
int ll_append(MetaInformation* mi, void* data) {
  MetaNode* n;
  MetaNode* beforeTail;

  /* Sanity check */
  if(mi == NULL) {
    return LL_MI_NULL_POINTER;
  }

  /* Temporarily store the node before the end to simplify the actions below */
  beforeTail = ((MetaNode*) mi->tail)->prev;

  /* Create the new node */
  n = malloc(sizeof(MetaNode));
  n->data = data;
  n->next = mi->tail;
  n->prev = beforeTail;

  /* Link it to the list */
  beforeTail->next = n;

  /* Tell the tail that there is a new node just before it */
  ((MetaNode*) mi->tail)->prev = n;

  return LL_OK;
}



/**********************************************************************
 * Name        : ll_first
 * Description : Return the first element in the list
 * Arguments   :
 * Returning   :
 * Precond     :
 * Postcond    :
 **********************************************************************/
MetaNode* ll_first(MetaInformation* mi) {
  return mi->head;
}


/**********************************************************************
 * Name        : ll_last
 * Description : return the last element in the list
 * Arguments   :
 * Returning   :
 * Precond     :
 * Postcond    :
 **********************************************************************/
MetaNode* ll_last(MetaInformation* mi) {
  return mi->tail->prev;
}


MetaNode* ll_get(MetaInformation* mi, int number) {
  MetaNode* p;
  int i;

  /* Sanity checks */
  if(mi == NULL) {
    return (MetaNode*) LL_MI_NULL_POINTER;
  }

  i=0;
  p = mi->head->next;
  while(p != mi->tail) {
    if(i == number)
      return p;
    /* Go to the next node */
    p = p->next;
    i++;
  }
  return NULL;
}

/**********************************************************************
 * Name        : ll_removeindex
 * Description : 
 * Arguments   :
 * Returning   :
 * Precond     :
 * Postcond    :
 **********************************************************************/
int ll_removeindex(MetaInformation* mi, int index) {
  MetaNode* p;
  int i;

  /* Sanity checks */
  if(mi == NULL) {
    return LL_MI_NULL_POINTER;
  }

  i=0;
  p = mi->head->next;
  while(p != mi->tail) {
    /* Execute the callback function */
    if(i==index) {
      /* Remove the node */
      ((MetaNode*) p->prev)->next = p->next;
      ((MetaNode*) p->next)->prev = p->prev;
      free(p);
      return 0;
    }
    /* Go to the next node */
    p = p->next;
    i++;
  }
  return -1;
}


/**********************************************************************
 * Name        : ll_traverse_with_function
 * Description : Traverse the list
 * Arguments   :
 * Returning   :
 * Precond     :
 * Postcond    :
 **********************************************************************/
int ll_traverse_with_function(MetaInformation* mi, void function(void*)) {
  MetaNode* p;

  /* Sanity checks */
  if(mi == NULL) {
    return LL_MI_NULL_POINTER;
  }

  if(function == NULL) {
    return LL_FUNCTION_NULL_POINTER;
  }

  p = mi->head->next;
  while(p != mi->tail) {
    /* Execute the callback function */
    (function)(p->data);
    /* Go to the next node */
    p = p->next;
  }

  return LL_OK;
}



/**********************************************************************
 * Name        : ll_remove
 * Description : Only removes one instance of the supplied data
 *               Returns 1 if node removed otherwise 0
 * Arguments   :
 * Returning   :
 * Precond     :
 * Postcond    :
 **********************************************************************/
int ll_remove(MetaInformation* mi, int function(void*, void*), void* data) {
  MetaNode* p;

  /* Sanity checks */
  if(mi == NULL) {
    return LL_MI_NULL_POINTER;
  }

  if(function == NULL) {
    return LL_FUNCTION_NULL_POINTER;
  }

  p = mi->head->next;
  while(p != mi->tail) {
    /* Execute the callback function */
    if((function)(p->data, data) == 1) {
      /* Remove the node */
      ((MetaNode*) p->prev)->next = p->next;
      ((MetaNode*) p->next)->prev = p->prev;
      free(p);
      return 1;
    }
    /* Go to the next node */
    p = p->next;
  }
  return 0;
}
