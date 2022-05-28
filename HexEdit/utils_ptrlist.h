/***************************************************************************
 *        utils_ptrlist.h  -  description
 *
 *  General Pointer list data structure and operation.                                                                         *
 ***************************************************************************/

#ifndef _PTRLIST_H_		// Use same macro name as one in hp_ptrlist.h
#define _PTRLIST_H_		// so that only one of them can be included.

#include <search.h>
/* For use with hsearch(3).  */
#ifndef __COMPAR_FN_T
# define __COMPAR_FN_T
typedef int (*__compar_fn_t) (const void *, const void *);

# ifdef __USE_GNU
typedef __compar_fn_t comparison_fn_t;
# endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tagPtrNode PtrNode;

#undef POSITION
typedef PtrNode* POSITION;

struct tagPtrNode {
	void*		ptr;
	POSITION	prev, next;
};

#define PTRNODE_INITIALIZER	{NULL,NULL,NULL}

typedef struct tagPtrList {
	int		count;
	POSITION	head, rear;
	__compar_fn_t	ptrcmp;		// -1: ptr1 < ptr2, 0: ptr1==ptr2, 1: ptr1 > ptr2
} PtrList;

#define	PTRLIST_INITIALIZER	{0,NULL,NULL,NULL}

/* initialize/terminate */
extern PtrList* PtrList_new( void );
//extern void PtrList_delete(PtrList *list);	// same name use for new purpose
extern void PtrList_set_compare( PtrList *list, __compar_fn_t cmpfunc );
extern void PtrList_initialize( PtrList *list );
extern void PtrList_terminate( PtrList *list );

/* get count */
#define PtrList_get_count(ptrlist)		( (ptrlist)->count)

/* get position */
#define PtrList_get_head(ptrlist)			( (ptrlist)->head )
#define PtrList_get_rear(ptrlist)			( (ptrlist)->real )

extern POSITION PtrList_find(PtrList *list, void *ptr);

/* get all node's pointer value */
extern int PtrList_get_all(PtrList *list, void* *buf, int buf_size);

#define PtrNode_get_next(node)		( (node)->next )
#define PtrNode_get_prev(node)		( (node)->prev )
#define PtrNode_get(node)					( (node)->ptr )

/* forward and backward travel along the list */
extern POSITION PtrNode_backward(POSITION pos, int step);
extern POSITION PtrNode_forward(POSITION pos, int step);

/* insert/append */
extern POSITION PtrList_insert_before(PtrList *list, POSITION node, void *ptr);
extern POSITION PtrList_insert_after(PtrList *list, POSITION node, void *ptr);
#define PtrList_append(list, ptr)	PtrList_insert_after( list, NULL, ptr )
#define PtrList_prepend(list, ptr)	PtrList_insert_before( list, NULL, ptr )

extern POSITION PtrList_insert_1to9(PtrList *list, void *ptr);
extern POSITION PtrList_insert_9to1(PtrList *list, void *ptr);

/* remove */
extern void* PtrList_remove(PtrList *list, POSITION node);
#define PtrList_remove_head(list)	PtrList_remove(list, (list)->head)
#define PtrList_remove_rear(list)	PtrList_remove(list, (list)->rear)
extern void PtrList_remove_all(PtrList *list);

/* delete -- remove and delete the node body */
extern void PtrList_delete(PtrList *list, POSITION node);
#define PtrList_delete_head(list)		PtrList_delete(list, (list)->head)
#define PtrList_delete_rear(list)		PtrList_delete(list, (list)->rear)
extern void PtrList_delete_all(PtrList *list);


/* ---------------------------
 * stack style: PUSH, POP
 * ---------------------------*/
typedef PtrList PtrStack;

#define PtrStack_peek( list )		( (list)->count > 0 ? (list)->head->ptr : NULL )
#define PtrStack_push( list, ptr )	PtrList_insert_before( list, NULL, ptr )
#define PtrStack_pop( list )		PtrList_remove( list, (list)->head )
/* ---------------------------
 * string operation
 * ---------------------------*/
typedef PtrList StrList;

extern POSITION StrList_find( StrList *list, const char *string );	// case insensitive
extern POSITION StrList_exact_find( StrList *list, const char *string ); // case sensitive

extern POSITION StrList_insert_string_atoz( StrList *list, const char *string );
extern POSITION StrList_insert_string_ztoa( StrList *list, const char *string );
extern void	StrList_remove_all( StrList *list );

#define PtrList_find_string(list, str) 		StrList_exact_find( list, str )
#define PtrList_insert_string_atoz(list, str)	StrList_insert_string_atoz( list, str )
#define PtrList_insert_string_ztoa(list, str)	StrList_insert_string_ztoa( list, str )

/* ---------------------------
 * string mapping operation
 * ---------------------------*/
typedef PtrList StrMap;

typedef struct tagStrStrPair {
	char *key;
	char *value;
} StrStrPair;

/* return: NULL if not found */
extern const char * StrMap_get( StrMap *strmap, const char * key );
extern const char * StrMap_safe_get( StrMap * strmap, const char * key, const char * default_value );

/* param: 'value' can be NULL, to remove specified header;
 * return: number of headers */
extern int  StrMap_set( StrMap *strmap, const char *key, const char * value );
extern void StrMap_prepend( StrMap *strmap, const char *key, const char *value );

extern void StrMap_remove( StrMap * strmap, const char * key );
extern void StrMap_remove_all( StrMap * strmap );

#ifdef __cplusplus
};
#endif

#endif /* _UTILS_PTRLIST_H_ */
