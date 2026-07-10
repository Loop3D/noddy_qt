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
 *	File:		3dCube.h
 *
 *	© 1989 Mark M. Owen.  All rights reserved.
 *****************************************************************************
 */

#ifndef	_3dCube_
#define	_3dCube_

#if (XVTWS != XOLWS)
#include "3dSolids.h"
#endif

#if XVT_CC_PROTO
int SolidUnitCube(pGroup, int, RendAttr, Matrix3D*);
#else
int SolidUnitCube();
#endif

#endif
