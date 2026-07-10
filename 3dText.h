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
 *	File:		3dText.h
 *	© 1989 Mark M. Owen.  All rights reserved.
 *****************************************************************************
 */
#ifndef _3dText_
#define _3dText_

#include "3dBitmap.h"

enum alignment	{ leftAlign,rightAlign,centerAlign };

#if XVT_CC_PROTO
int StringTo3d (char*, Point3d**, int*);
void DrawString3d (Fixed, Fixed, Fixed, char*, enum alignment);
#else
int StringTo3d ();
void DrawString3d ();
#endif

#endif
