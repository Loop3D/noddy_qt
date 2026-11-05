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
 *	File:		3dSolids.h
 *
 *	© 1989 Mark M. Owen. All rights reserved.
 *****************************************************************************/

#ifndef _3dSolids_
#define	_3dSolids_

#include <math.h>
#if (XVTWS != XOLWS)
#include "3dHier.h"
#endif

#if XVT_CC_PROTO
Fixed Distance (Point3d);
int xfRenderGroup (pGroup, Matrix3D*, pLighting, BOOLEAN);
int xfRenderCollection (pCollection, Matrix3D*, pLighting, BOOLEAN);
#else
Fixed Distance ();
int xfRenderGroup ();
int xfRenderCollection ();
#endif

#endif

