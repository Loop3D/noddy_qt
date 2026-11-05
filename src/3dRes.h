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
 *	File:		3dRsources.h
 *
 *	© 1989 Mark M. Owen. All rights reserved.
 *
 *****************************************************************************
 */

#ifndef _3dResources_
#define _3dResources_

#if (XVTWS != XOLWS)
#include "3dHier.h"
#endif

#define	CLCT	"CLCT"
#define	GRUP	"GRUP"
#define	PACH	"PACH"
#define	FACT	"FACT"
#define	PVRT	"PVRT"

typedef	struct
{
	short	nG;
	short	*idG;
} CLCT_res,*pCLCT_res,**hCLCT_res;

typedef struct
{
	short	nP;
	short	*idP;
} GRUP_res,*pGRUP_res,**hGRUP_res;

typedef struct
{
	BOOLEAN	instanced;
	Matrix3D	xf;
	RendAttr RA;
} PTCH_res,*pPTCH_res,**hPTCH_res;

typedef	struct
{
	short	nV;
	Vrtx	*V;
} PVRT_res,*pPVRT_res,**hPVRT_res;

typedef	struct
{
	short	nF;
	Facet	*F;
} FACT_res,*pFACT_res,**hFACT_res;


#if XVT_CC_PROTO
int WriteCollection (pCollection, char*, int);
int WriteGroup (pGroup, char*, int);
int WritePatch (pPatch, char*, int);

pCollection GetCollection (int, char*);
pGroup GetGroup (int, char*);
int GetPatch (pGroup, int, int);
#else
int WriteCollection ();
int WriteGroup ();
int WritePatch ();

pCollection GetCollection ();
pGroup GetGroup ();
int GetPatch ();
#endif

#endif     
