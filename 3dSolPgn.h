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
 *	File:		3dSolidsPgn.h
 *
 *	© 1991 Mark M. Owen. All rights reserved.
 *****************************************************************************
 */
#define	_H_3dSolidsPgn

#ifndef _3dSolidsPgn_
#define	_3dSolidsPgn_

#include "3dSolids.h"
#include "3dTextur.h"
#include "3dImage.h"

#if XVT_CC_PROTO
int xfRenderZBuffer(pCollection pC, Matrix3D *xf, pLighting pL, pZBuffer pLZB);
int xfRenderCollectionPgn(pCollection pC, Matrix3D *xf, pLighting pL);
int xfRenderCollectionImage(pCollection pC, Matrix3D *xf, pLighting pL, pImage pI);
#else
int xfRenderZBuffer();
int xfRenderCollectionPgn();
int xfRenderCollectionImage();
#endif

#endif /* _3dSolidsPgn_ */

