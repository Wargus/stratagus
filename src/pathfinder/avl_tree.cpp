
/* $Id$ */

#include <stdio.h>
#include <stdlib.h>

#include "stratagus.h"
//#define PRINT_TREE
#include "avl_tree.h"
#ifdef PRINT_TREE
#include <ctype.h>      /* isspace() */
#endif

enum subtrees {
	LEFT,
	RIGHT
};

local void AvlDestroyNodes (AvlTreeNode * );
local AvlTreeNode *AvlNodeNew (void * , int );
local AvlTreeNode *rotate (AvlTreeNode * , int );

void AvlFlush (AvlTree *root)
{
	AvlDestroyNodes (root->subtree[RIGHT]);
	root->subtree[RIGHT] = NULL;
}

local void AvlDestroyNodes (AvlTreeNode *root)
{
	if (root) {
				AvlDestroyNodes (root->subtree[LEFT]);
				AvlDestroyNodes (root->subtree[RIGHT]);
				free (root->data);
				free (root);
		}
}

local AvlTreeNode *AvlNodeNew (void *data, int key)
{
		AvlTreeNode *new;

		new = (AvlTreeNode * )malloc (sizeof (AvlTreeNode));
		if (!new)
				return NULL;

		new->data = data;
		new->key = key;
		new->balance = 0;
		new->subtree[LEFT] = new->subtree[RIGHT] = NULL;
		return new;
}

void AvlAdd (AvlTree *root, void *data, int key)
{
		AvlTreeNode *new;
		AvlTreeNode *imbalanced, *imbalanced_parent, *imbalanced_child;
		AvlTreeNode *current_parent, *current;
		int lastdir;
		int dir;

		if (!root->subtree[RIGHT]) {
				root->subtree[RIGHT] = AvlNodeNew (data, key);
				return;
		}

		/* find the right place */
		imbalanced = current = root->subtree[RIGHT];
		imbalanced_parent = root;
		lastdir = RIGHT;
		while (1) {
				current_parent = current;
				if (key < current->key) {
						current = current->subtree[LEFT];
						lastdir = LEFT;
				} else if (key > current->key) {
						current = current->subtree[RIGHT];
						lastdir = RIGHT;
				} else				/* it's already there */
						return;

				if (current) {
						if (current->balance != 0) {
								imbalanced = current;
								imbalanced_parent = current_parent;
						}
				} else {
						new = AvlNodeNew (data, key);
						current_parent->subtree[lastdir] = current = new;
						break;
				}
		}

		/* the new node is inserted -> adjust balance factors along the way */
		if (new->key < imbalanced->key)
				dir = LEFT;
		else
				dir = RIGHT;
		/* dir now indicates which of both subtrees of the potentially imbalanced
		 * node the new node was added to */
		current = imbalanced;
		imbalanced_child = imbalanced->subtree[dir];
		while (current != new) {
				if (new->key < current->key) {
						current->balance += -1;
						current = current->subtree[LEFT];
				} else {
						current->balance += 1;
						current= current->subtree[RIGHT];
				}
		}

		if (abs (imbalanced->balance) > 1) {
				int otherdir = 1 ^ dir;

				if (imbalanced->balance * imbalanced_child->balance > 0) {
						/* both balance factors have the same sign => single rotation */
						current = rotate (imbalanced, otherdir);
				} else {
						/* double rotation */
						imbalanced->subtree[dir] = rotate (imbalanced_child, dir);
						current = rotate (imbalanced, otherdir);
				}
				if (imbalanced_parent->subtree[LEFT] == imbalanced)
						imbalanced_parent->subtree[LEFT] = current;
				else
						imbalanced_parent->subtree[RIGHT] = current;
		}
}

void *AvlDelete (AvlTree *root, int key)
{
		AvlTreeNode *ptr;
		AvlTreeNode *stack[20];				/* stack of nodes traversed */
		unsigned char dir[20];				/* stack[i]->subtree[dir[i]] == stack[i+1] */
		int k;
		void *retval;

		ptr = root->subtree[RIGHT];
		stack[0] = root;
		dir[0] = RIGHT;				/* by definition - the real tree root is the right
												 * subtree of dummy root node */
		k=1;
		do {
				if ( !ptr ) {
						//printf ("avl_delete: not found\n");
						return NULL;						/* not found */
				}
				if (key < ptr->key) {
						dir[k] = LEFT;
						stack[k++] = ptr;
						ptr = ptr->subtree[LEFT];
				} else if (key > ptr->key) {
						dir[k] = RIGHT;
						stack[k++] = ptr;
						ptr = ptr->subtree[RIGHT];
				} else
						break;
		} while (1);

		/* let's see how many subtrees we'll have to deal with after we delete
		 * the node */
		if (ptr->subtree[LEFT] == NULL || ptr->subtree[RIGHT] == NULL) {
				AvlTreeNode *child;

				/* zero or one subtree */
				if (ptr->subtree[LEFT] == NULL) {
						child = ptr->subtree[RIGHT];
				/* JOHNS: See above if this must be true!
				} else if (ptr->subtree[RIGHT] == NULL) {
				*/
				} else {
						child = ptr->subtree[LEFT];
				}
				retval = ptr->data;
				free (ptr);
				stack[k-1]->subtree[dir[k-1]] = child;
		} else {
				/* both non-NULL */
				/* We just continue the traversal pushing every node visited on
				 * stack until we find the leftmost node of our node's right subtree.
				 * We store the pointer to the node being deleted to stack[k] but
				 * we mark this position in l so that after the inorder successor is
				 * found it can take the deleted node's place on stack (as it did in
				 * the actual tree). */
				int l=k;
				dir[k] = RIGHT;
				stack[k++] = ptr;				/* this is just temporary */
				/* find the rightmost node in the left subtree of the node being
				 * deleted and store it in ptr */
				ptr=ptr->subtree[RIGHT];
				while (ptr->subtree[LEFT]) {
						dir[k] = LEFT;
						stack[k++] = ptr;
						ptr = ptr->subtree[LEFT];
				}
				/* ptr->subtree[LEFT] is now guaranteed to be NULL */
				/* replace the node being deleted by its inorder successor and fix
				 * all the pointers concerned by this */
				stack[k-1]->subtree[dir[k-1]] = ptr->subtree[RIGHT];
				stack[l-1]->subtree[dir[l-1]] = ptr;
				ptr->subtree[LEFT] = stack[l]->subtree[LEFT];
				ptr->subtree[RIGHT] = stack[l]->subtree[RIGHT];
				/* just for now, it gets revised during rebalancing stage */
				ptr->balance = stack[l]->balance;
				retval = stack[l]->data;
				free (stack[l]);
				stack[l] = ptr;
		}

		/* walk through the stack node after node and rebalance if needed */
		while (--k) {
				int thisdir = dir[k];
				int otherdir = 1 ^ thisdir;
				int balance_change = 2*otherdir -1;
				/* e.g. if dir[k]==LEFT, it means that stack[k]'s LEFT subtree has been
				 * diminished by the deletion. Thus its balance swings to the RIGHT
				 * (this is expressed by balance_change = 2*otherdir -1 which is either
				 * 1 or -1). Now we need to find out what this will do to the node's
				 * balance. */

				if (stack[k]->balance == 0) {
						/* the balance was 0 so adding 1 or -1 cannot give illegal result */
						stack[k]->balance += balance_change;
						break;
				} else if (stack[k]->balance == -balance_change) {
						/* new balance will be 0 */
						stack[k]->balance += balance_change;
				} else {				/* stack[k]->balance == balance_change */
						/* if dir[k]==LEFT then stack[k]'s LEFT subtree has been diminished,
						 * resulting in imbalance of stack[k]. What kind of rotation will
						 * be needed is determined by the balance of stack[k]'s other
						 * (RIGHT is this case) subtree root, which is r */
						AvlTreeNode *r = stack[k]->subtree[otherdir];

						/* stack[k]->balance==balance_change so this sends the node's
						 * balance out of bounds */
						stack[k]->balance += balance_change;
						/* rebalance */
						if (r->balance == -balance_change) {
								/* if r leans to the other side than stack[k] we need to
								 * rotate twice */
								stack[k]->subtree[otherdir] = rotate (r, otherdir);
								stack[k-1]->subtree[dir[k-1]] = rotate (stack[k], thisdir);
						} else {
								/* if r leans to the same side as stack[k] or is balanced,
								 * one rotation will do */

								/* this is needed because rotate() can change r's balance */
								int r_old_balance = r->balance;
								stack[k-1]->subtree[dir[k-1]] = rotate (stack[k], thisdir);
								if (r_old_balance == 0) {
										break;
								}
						}
				}
		}
		return retval;
}

/* computation of new balance factors might be non-trivial so here's a short
 * explanation:
 * Consider this situation:
 *
 *						+---+
 *			   +--------| S |-----+
 *			   |		+---+	 |
 *			 +---+				|
 *		+----| R |---+		   +-+
 *		|	+---+   |		   | |
 *	   +-+		  +-+		  |c|
 *	   | |		  | |		  | |
 *	   |a|		  |b|		  +-+
 *	   | |		  | |
 *	   +-+		  +-+
 *
 * We want to rotate this to the right so that R is the new root, S is its
 * right child and right subtree of R (b) becomes left subtree of S. Balance
 * factors of R and S are known. What will the new balance factors of R and S
 * be after the rotation?
 *
 * The new balance factor of S will be height(c)-height(b), R will end up with
 * height(S)-height(a) where height(S)=1+max(height(c),height(b). Say that the
 * height of the c subtree is h. It is clear that if we can determine heights
 * of a, b and c (their *differences* are enough) the new balance factors can
 * be computed easily.
 *
 * Let's put height(c)=h. Then height of S's left subtree is h-balance(S). This
 * means that height of R's taller subtree must be h-balance(S)-1. Which of
 * both R's subteees is taller is determined by sign of balance(R). If
 * balance(R) > 0 then b is taller than a and its height is h-balance(S)-1.
 * Now that we have height(b) we can determine height(a) as
 * height(b)-balance(R). It can be written like this:
 *
 * height(c)=h		 (axiom)
 * height(b)=h-balance(S)-1 + (balance(R)<=0 ? balance(R) : 0)
 * height(a)=h-balance(S)-1 - (balance(R)>=0 ? balance(R) : 0)
 *
 * newbalance(S) = height(c)-height(b) =
 *			   = balance(S)+1-(balance(R)<0 ? balance(R) : 0)
 * newbalance(R) = 1+max(height(c),height(b)) - height(a) =
 *			   = 1+ (newbalance(S)>=0 ? height(c) : height(b)) - height(a) =
 *			   = 1+ (newbalance(S)>=0 ? height(c)-height(a) : balance(R)) =
 *			   = 1+ (newbalance(S)>=0 ? balance(S)+1 +
 *							   (balance(R)>=0 ? balance(R) : 0) : balance(R)
 *
 * All of this holds for rotation to the right. In order for this to work for
 * left rotation as well, it turns out that you need to invert signs of
 * balance factors of S and R before the computation and then invert signs
 * of new balance factors, too.
 */

local AvlTreeNode *rotate (AvlTreeNode *root, int dir)
{
		int otherdir = 1 ^ dir;
		/* the caller verified that r exists */
		AvlTreeNode *r = root->subtree[otherdir];
		char br, bs, nbr, nbs;

		root->subtree[otherdir] = r->subtree[dir];
		r->subtree[dir] = root;

		/* compute new balance factors */
		br = dir==RIGHT ? r->balance : -r->balance;
		bs = dir==RIGHT ? root->balance : -root->balance;
		nbs = bs + 1 - (br<0 ? br : 0);
		nbr = (nbs>=0 ? bs + 1 + (br>0 ? br : 0) : br) + 1;
		r->balance = dir==RIGHT ? nbr : -nbr;
		root->balance = dir==RIGHT ? nbs : -nbs;
		return r;
}

void *AvlFind (AvlTree *root, int key)
{
		AvlTreeNode *ptr;

		for (ptr = root->subtree[RIGHT]; ptr; ) {
				if (key < ptr->key)
						ptr=ptr->subtree[LEFT];
				else if (key > ptr->key)
						ptr=ptr->subtree[RIGHT];
				else
						return ptr->data;
		}
		return NULL;
}




#ifdef PRINT_TREE

/* PRINTING OF A TREE */

struct line {
		char *free;
		char *line;
};
#define PRINTED_LINES 20
static struct line line[PRINTED_LINES];
static int seqno;		/* sequence number of node in ordered tree */
static int llen=78;		/* length of line */

static void avl_print_prepare (struct avl_tree_node * , int );
static void avl_print_reset (void);
static void avl_print_tree_node (struct avl_tree_node * , int );
static void avl_print_joints (struct avl_tree_node * , int );
static void avl_print_tree_joint (struct avl_tree_node * , int );

void AvlPrint (AvlTree *root)
{
		int i;

		seqno = 0;
		avl_print_prepare (root->subtree[RIGHT], 0);
		avl_print_joints (root->subtree[RIGHT], 0);
		for (i=0; i<PRINTED_LINES; i++) {
				if (line[i].line)
						printf ("%s\n", line[i].line);
		}
		avl_print_reset ();
}

static void avl_print_prepare (struct avl_tree_node *root, int level)
{
		if (!root)
				return;
		avl_print_prepare (root->subtree[LEFT], level+1);
		root->seqno = seqno++;
		avl_print_tree_node (root, level);
		avl_print_prepare (root->subtree[RIGHT], level+1);
}

static void avl_print_tree_node (struct avl_tree_node *n, int level)
{
		short free;

		level = 2*level + 1;

		if (line[level].line == NULL) {
				line[level].line = line[level].free = malloc (llen);
				memset (line[level].line, ' ', llen);
				line[level].line[llen-1] = '\0';
		}
		if (line[level].free - line[level].line >= llen-1)
				return;				/* this line is full already */

		if (n->seqno * 8 > llen-2)
				return;				/* can't write behind the line's end */
		*line[level].free = ' ';		/* erase the trailing '\0' */
		line[level].free = line[level].line + n->seqno * 8;

		free = llen-1-(line[level].free-line[level].line);
		free = free<0 ? 0 : free;
		n->entrylen = snprintf (line[level].free, free, "%d:%d", n->key,n->balance);
		line[level].free += n->entrylen;
		if (line[level].free - line[level].line > llen-1)
				line[level].free = line[level].line + llen-1;

		free = llen-1-(line[level].free-line[level].line);
		free = free<0 ? 0 : free;
		line[level].free += snprintf (line[level].free, free, " ");
		if (line[level].free - line[level].line > llen-1)
				line[level].free = line[level].line + llen-1;
}

static void avl_print_joints (struct avl_tree_node *root, int level)
{
		if (!root)
				return;
		avl_print_joints (root->subtree[LEFT], level+1);
		avl_print_tree_joint (root, level);
		avl_print_joints (root->subtree[RIGHT], level+1);
}

static void avl_print_tree_joint (struct avl_tree_node *n, int level)
{
		int this = n->seqno * 8;
		int i;

		level = 2*level;

		if (line[level].line == NULL) {
				line[level].line = line[level].free = malloc (llen);
				memset (line[level].line, ' ', llen);
				line[level].line[llen-1] = '\0';
		}
		if (this + n->entrylen/2 < llen-1)
				line[level].line[this + n->entrylen/2] = '|';

		level++;

		if (n->subtree[LEFT]) {
				int left = 8 * n->subtree[LEFT]->seqno + n->subtree[LEFT]->entrylen/2;
				if (left < llen-2)
						line[level].line[left] = '+';
				for (i=left+1; i<this-1 && i<llen-2; i++)
						line[level].line[i] = '-';
		}

		if (n->subtree[RIGHT]) {
				int right = 8*n->subtree[RIGHT]->seqno + n->subtree[RIGHT]->entrylen/2;
				int truncated=0;

				if (right > llen-2) {
						right = llen-2;
						truncated = 1;
				}
				for (i=this; line[level].line[i] && !isspace(line[level].line[i]); i++);
				i++;
				for ( ; i<right; i++)
						line[level].line[i] = '-';
				if ( !truncated )
						line[level].line[right] = '+';
		}
}

static void avl_print_reset (void)
{
		int i;

		for (i=0; i<PRINTED_LINES; i++) {
				if (line[i].line)
						free (line[i].line);
				line[i].free = line[i].line = NULL;
		}
}

#endif /* PRINT_TREE */
