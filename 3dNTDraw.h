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
*	file:		"3dNTDrawing.h"
*
*	purpose:	prototypes for non-transformed drawing routines.
*
*	©1990 Mark M. Owen.  All rights reserved.
*****************************************************************************/
#ifndef _3dNTDrawing_
#define	_3dNTDrawing_

#include "3dClip.h"

#if XVT_CC_PROTO
void ntDrawLine3d (Point3d *, Point3d *);
#else
void ntDrawLine3d ();
#endif

#endif
