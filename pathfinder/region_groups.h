
/* $Id$ */

#ifndef REGION_GROUPS_H
#define REGION_GROUPS_H

int RegGroupsInitialize (void);
int RegGroupSetInitialize (void);
int RegGroupCheckConnectivity (Unit * , int , int );
void RegGroupDestroy (int );
int SuperGroupGetNumRegions (Unit * , int );
void RegGroupIdMarkRegions (int , Region * );
void RegGroupIdDeleteRegion (int , Region * );
void RegGroupIdAddRegion (int , Region * );
void RegGroupConsistencyCheck (int );
void RegGroupAddToSuperGroup (int );

#endif /* REGION_GROUPS_H */
