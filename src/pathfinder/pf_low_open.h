
/* $Id$ */

#ifndef LOW_OPEN_H
#define LOW_OPEN_H

extern int LowOpenInit (int );
extern void LowOpenReset (void);
extern void LowOpenAdd (MapField * );
extern void LowOpenDelete (MapField * );
extern MapField *LowOpenGetFirst (void);
//#ifdef PRINTING
extern void LowOpenPrint (void);
//#endif

#endif /* LOW_OPEN_H */
