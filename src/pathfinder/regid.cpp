
/* $Id$ */

#include <stdio.h>
#include <stdlib.h>

#include "regid.h"

void RegidSpaceInitialize (RegidSpace *space, int size)
{
	space->RegidHigh = 0;
	space->BitmapSize = size;
	if (space->Bitmap)
		free (space->Bitmap);
	space->Bitmap = (unsigned char * )calloc (size, sizeof (unsigned char));
}

void RegidSpaceDestroy (RegidSpace *space)
{
	if (space->Bitmap)
		free (space->Bitmap);
}

void RegidBitmapInflate (RegidSpace *space)
{
	space->Bitmap = realloc (space->Bitmap, ++space->BitmapSize);
	/* initialize the newly acquired memory */
	space->Bitmap[space->BitmapSize - 1] = 0;
}

void RegidBitmapShrink (RegidSpace *space)
{
	unsigned int size_needed;

	size_needed = space->RegidHigh/8;
	if (space->RegidHigh%8)
		++size_needed;

	if (size_needed < space->BitmapSize) {
		space->Bitmap = realloc (space->Bitmap, size_needed);
		space->BitmapSize = size_needed;
	}
}

int RegidInUse (RegidSpace *space, unsigned int regid)
{
	--regid;		/* regions are numbered starting at 1, not 0 */
	return space->Bitmap[regid/8] & (1 << regid%8);
}

void RegidMarkUnused (RegidSpace *space, unsigned int regid)
{
	space->Bitmap[(regid-1)/8] &= ~(1 << (regid-1)%8);
	if (regid == space->RegidHigh) {
		/* we are destroying the highest numbered region so we need
		 * to find a new one */
		space->RegidHigh = RegidFind (space, REGID_HIGHEST, REGID_USED);
	}
}

void RegidMarkUsed (RegidSpace *space, unsigned int regid)
{
	space->Bitmap[(regid-1)/8] |= 1 << (regid-1)%8;
	if (regid > space->RegidHigh)
		space->RegidHigh = regid;
}

int RegidFind (RegidSpace *space, enum regid_dir dir, enum regid_usage used)
{
	unsigned int byte;
	int increment;

	if (dir==REGID_LOWEST) {
		increment = 1;
		byte = 0;
	} else {
		increment = -1;
		byte = space->BitmapSize - 1;
	}

	for ( ; byte >=0 && byte < space->BitmapSize; byte += increment) {
		int i;

		/* a little optimization - we check for the easy cases first */
		if (used == REGID_USED && space->Bitmap[byte] == 0) {
			/* this byte doesn't have any used regids (should not happen?) */
			continue;
		} else if (used == REGID_UNUSED && space->Bitmap[byte] == 0xff) {
			/* this byte doesn't have any unused regids */
			continue;
		}
		/* the above didn't work, we have to do a bit-by-bit search */
		i = dir==REGID_LOWEST ? 0 : 7;
		for ( ; i<8 && i>=0; i += increment) {
			if ( (space->Bitmap[byte] & (1 << i)) == used)
				return 8*byte + i + 1;
		}
	}
	return -1;
}
