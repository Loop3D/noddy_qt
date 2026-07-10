/* ============================================================================
 * [XVT -> Qt PORT] This file is unchanged calculation/application logic.
 * Its XVT toolkit calls now run against the Qt5-backed compatibility layer
 * in qt_compat/ (see qt_compat/xvt.h and CLAUDE.md) instead of the original
 * proprietary XVT SDK. No calculation or algorithmic code in this file was
 * modified to accomplish this port -- only the underlying GUI toolkit
 * implementation, in qt_compat/, changed.
 * ============================================================================
 */
/*****************************************************************************
 *	File:		3dZBuff.h
 *
 *	© 1991 Mark M. Owen. All rights reserved.
 *****************************************************************************
 */
#define	_H_ZBuffers

#ifndef	_ZBuffers_
#define	_ZBuffers_

typedef struct
{
	long	*zb;			/*	Z depth values		*/
	long	xMn, xMx, yMn, yMx;	/*	bounds of the Z buffer	*/
	long	xRng,yRng;		/*	bounds spans		*/
	RCT content;			/*	active area of buffer	*/
} ZBuffer, *pZBuffer;

#if XVT_CC_PROTO
void	AllocZB (pZBuffer pzb, long left, long top, long right, long bottom);
void	DeallocZB (pZBuffer pzb);
long	GetZB (pZBuffer pzb, long x, long y);
BOOLEAN SetZB (pZBuffer pzb, long x, long y, long v);
void	BitMapFromZB(pZBuffer pzb, WINDOW gp);
#else
void	AllocZB ();
void	DeallocZB ();
long	GetZB ();
BOOLEAN SetZB ();
void	BitMapFromZB();
#endif

#endif	/* _ZBuffers_ */

