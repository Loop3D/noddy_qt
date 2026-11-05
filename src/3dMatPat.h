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
 *	File:		3dMatrix2Patch.h
 *
 *	© 1989 Mark M. Owen.  All rights reserved.
 *****************************************************************************
 */

#ifndef	_3dMatrix2Patch_
#define	_3dMatrix2Patch_

#if (XVTWS != XOLWS)
#include "3dSolids.h"
#endif

#if XVT_CC_PROTO
int xfMatrix2Patch(pGroup, int, int, int, int, int, Point3d __huge *, RendAttr, Matrix3D*);
#else
int xfMatrix2Patch();
#endif

#endif               
