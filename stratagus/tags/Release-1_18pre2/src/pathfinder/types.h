
/* $Id$ */

#ifndef TYPES_H
#define TYPES_H

struct _coordinates_ {
	short X, Y;
};

typedef struct _coordinates_ AreaCoords;
typedef struct _coordinates_ FieldCoords;

typedef struct {
	int XMin, XMax, YMin, YMax;
} RectBounds ;

#endif /* TYPES_H */
