
/* $Id$ */

#ifndef REGID_H
#define REGID_H

enum regid_dir {
	REGID_LOWEST,
	REGID_HIGHEST
};
enum regid_usage {
	REGID_UNUSED,
	REGID_USED
};

typedef struct regid_space {
	unsigned short RegidHigh;
	unsigned short BitmapSize;
	unsigned char *Bitmap;
} RegidSpace;

extern void RegidSpaceInitialize (RegidSpace * , int );
extern void RegidBitmapInflate (RegidSpace * );
extern void RegidBitmapShrink (RegidSpace * );
extern int  RegidInUse (RegidSpace * , int );
extern void RegidMarkUnused (RegidSpace * , int );
extern void RegidMarkUsed (RegidSpace * , int );
extern int  RegidFind (RegidSpace * , enum regid_dir , enum regid_usage );

#endif /* REGID_H */
