
/* $Id$ */

#ifndef HIGHLEVEL_H
#define HIGHLEVEL_H

typedef enum _area_corners_ {
	UL, UR, LL, LR, NUM_CORNERS
} AreaCorners;

struct highlevel_path_step {
	/* Sequence number of this HighPathStep along the path. Two steps can have
	 * the same SeqNum if they were added to the path during the highlevel
	 * traceback to "fill up" the "corners" when two "true" on-path regions
	 * lie in 8-connected areas (see HighTraceback()).
	 */
	int SeqNum;
	unsigned short RegId;
	int H[NUM_CORNERS];
};
typedef struct highlevel_path_step HighPathStep;

struct highlevel_path {
	int NumSteps;
	HighPathStep *Sequence;
	unsigned char *Set;
	unsigned int OriginalGoalReachable:1;
	unsigned int Studied:1;
};
typedef struct highlevel_path HighlevelPath;

extern int HighlevelInit (void);
extern void HighlevelReset (void);
extern HighlevelPath *ComputeHighlevelPath (Unit * );
extern void HighReleasePath (Unit * );
extern void HighInvalidateCacheEntries (unsigned short );
extern void HighlevelSetGoalArea (int , int );
extern void HighlevelSetRegOnPath (int );
extern void HighlevelSetRegSeen (int );
extern void HighPrintStats (void);
extern void HighPrintPath (HighlevelPath * , int );

#endif /* HIGHLEVEL_H */
