
/* $Id$ */

#ifndef HIERARCHICAL_H
#define HIERARCHICAL_H

typedef enum node_operation {
    SET_REGID
} NodeOperation;

struct coordinates {
	short X, Y;
};

typedef struct coordinates AreaCoords;
typedef struct coordinates FieldCoords;

typedef enum area_neighborship_type {
	AREAS_NONCONNECTED,
	AREAS_8CONNECTED,
	AREAS_4CONNECTED
} AreaNeighborshipType;

extern int AreaGetWidth (void);
extern int AreaGetHeight (void);
extern int AreaNeighborship (AreaCoords * , AreaCoords * );
extern void NodePerformOperation (int , int , NodeOperation , int []);
extern int NodeGetAreaOffset (int , int );

#ifndef HIERARCHICAL_BACKEND

#include "unit.h"

extern int PfHierInitialize (void);
extern int PfHierComputePath (Unit * , int * , int * );

#endif /* HIERARCHICAL_BACKEND */

#endif /* HIERARCHICAL_H */
