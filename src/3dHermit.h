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
 *	File:	3dHermiteCurves.h
 *
 *	© 1989 Mark M. Owen.  All rights reserved.
 *****************************************************************************
 */
#ifndef	_3dHermiteCurves_
#define	_3dHermiteCurves_

#include "3d.h"

#define	HERMITE_OPENCURVE 				FALSE
#define	HERMITE_CLOSEDCURVE				TRUE
#define	HERMITE_ERROR_MISSING_ARGUMENTS	0x7FFF

/* OsErr	HermiteCurve3d	(Boolean,int,int,Point3d*,int*,Point3d**); */

#endif
