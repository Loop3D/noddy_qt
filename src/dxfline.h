/* ============================================================================
 * [XVT -> Qt PORT] This file is unchanged calculation/application logic.
 * Its XVT toolkit calls now run against the Qt5-backed compatibility layer
 * in qt_compat/ (see qt_compat/xvt.h and CLAUDE.md) instead of the original
 * proprietary XVT SDK. No calculation or algorithmic code in this file was
 * modified to accomplish this port -- only the underlying GUI toolkit
 * implementation, in qt_compat/, changed.
 * ============================================================================
 */
/*
 * ========================
 * NAME:
 *                      dxfline.h
 *
 * DESCRIPTION:
 *                      Include file for using the line module.
 *
 * AUTHOR:
 *                      Ewen Christopher
 *                      (c) Ewen Christopher, 1995.
 *                      (c) Monash University, 1995.
 * ========================
 */

#ifndef DXFLINE_H_INCLUDED
#define DXFLINE_H_INCLUDED



/**************** Data structures ********************/

struct DXFpoint                                                 /* One point / vertex */
{
        ftype point[3];                                         /* Coords of the point */
  struct DXFpoint *next;                /* Next point in list */
};

struct DXFline                                                  /* A list of points */
{
        struct DXFpoint *points;        /* Pointer to the list of points */
  struct DXFline *next;                 /* Next in list of lines */
};





/***************** Prototypes ********************/

#if XVT_CC_PROTO
void DXFlineRead( struct DXFhandle *handle, char *filename );
void DXFlineConv( struct DXFhandle *handle );


void dxfFreePoints( struct DXFpoint *points );
void dxfFreeLines( struct DXFline *line );
#else
void DXFlineRead();
void DXFlineConv();


void dxfFreePoints();
void dxfFreeLines();
#endif

#endif
