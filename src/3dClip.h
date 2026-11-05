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
*	file:		"3dClipping.h"
*
*	purpose:	Prototypes for 3d canonical clipping routines
*
*	©1990 Mark M. Owen.  All rights reserved.
*****************************************************************************/
#ifndef _3dClipping_
#define	_3dClipping_

#include "3dExtern.h"

#if XVT_CC_PROTO
void SetCanonicalClipping (BOOLEAN);
enum visibility ClipLine (Point3d *,Point3d *);
enum visibility ClipPt (Point3d *);
#else
void SetCanonicalClipping ();
enum visibility ClipLine ();
enum visibility ClipPt ();
#endif

#endif
