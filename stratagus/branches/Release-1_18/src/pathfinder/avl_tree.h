
/* $Id$ */

#ifndef AVL_TREE_H
#define AVL_TREE_H

typedef struct avl_tree_node {
	void *data;
	char balance;
	int key;
	struct avl_tree_node *subtree[2];
#ifdef PRINT_TREE
	char seqno;
	char entrylen;
#endif
} AvlTreeNode;

typedef AvlTreeNode AvlTree;

extern void AvlFlush (AvlTree * );
extern void AvlAdd (AvlTree * , void * , int );
extern void *AvlDelete (AvlTree * , int );
extern void *AvlFind (AvlTree * , int );
#ifdef PRINT_TREE
extern void AvlPrint (AvlTree * );
#endif /* PRINT_TREE */

#endif /* AVL_TREE_H */
