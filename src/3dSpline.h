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
 *	File:		3dBSplineCurves.h
 *
 *	© 1989 Mark M. Owen.  All rights reserved.
 *****************************************************************************
 */
#ifndef _3dBSplineCurves_
#define	_3dBSplineCurves_

#include "3d.h"

#define	BS_TOO_FEW_POINTS	-1

typedef struct
{
	FPType	a,b,c,d;
} BSCC;

typedef struct
{
	BSCC	x,y,z;
} BSC;


#if XVT_CC_PROTO
int BSpline3d	(int, int, Point3d _huge *, int*, Point3d _huge **);
#else
int BSpline3d	();
#endif

#endif
