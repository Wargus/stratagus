
/* $Id$ */

#ifndef HIGH_OPEN_H
#define HIGH_OPEN_H

extern int HighOpenInit (int );
extern void HighOpenReset (void);
extern void HighOpenAdd (Region * );
extern void HighOpenDelete (Region * );
extern Region *HighOpenGetFirst (void);

#endif /* HIGH_OPEN_H */
