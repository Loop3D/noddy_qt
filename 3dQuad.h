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
 *	File:		3dQuadratics.h
 *
 *	© 1990 Mark M. Owen.  All rights reserved.
 *****************************************************************************
 */

#ifndef _3dQuadratics_
#define _3dQuadratics_

#if (XVTWS != XOLWS)
#include "3dSolids.h"
#endif

#if XVT_CC_PROTO
typedef	void	(*pFunction)(Point3d, Point3d, Point3d, Point3d, Matrix3D*);

int xfWireFrameSphere (int, int, Fixed, Fixed, Matrix3D*, pFunction);
int SolidSphere (int, int, Fixed, Fixed, BOOLEAN, pGroup, int, RendAttr, Matrix3D*);
int SolidUnitSphere (Fixed segs, pGroup pG, int ixP, RendAttr raOptions, Matrix3D *xf);
int SolidCylinder (int, Fixed, Fixed, Fixed, Fixed, BOOLEAN, pGroup, int, RendAttr, Matrix3D*);
int SolidCone (int, Fixed, Fixed, Fixed, BOOLEAN, pGroup, int, RendAttr, Matrix3D*);
#else
typedef	void	(*pFunction)();

int xfWireFrameSphere ();
int SolidSphere ();
int SolidUnitSphere ();
int SolidCylinder ();
int SolidCone ();
#endif


#endif  
