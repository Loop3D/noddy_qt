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
 * =======================
 * NAME:
 *                      matrix.h
 *
 * DESCRIPTION:
 *                      Include file for using the <matrix> module of matrix manipulation
 *                      routines.  The routines are designed to assist in the construction
 *                      of transformation matricies for later application to the points
 *                      from a DXF file.
 *
 * AUTHOR:
 *                      Ewen Christopher
 *                      (c) Ewen Christopher 1994
 *                      (c) Monash University 1994
 * =======================
 */

void identity( float I[4][4] );
void multiplyMatrix( float result[4][4], const float a[4][4],
                                                                                 const float b[4][4] );

void rotateX( float result[4][4], float d );
void rotateY( float result[4][4], float d );
void rotateZ( float result[4][4], float d );

void translate( float result[4][4], float x, float y, float z );
void scale( float result[4][4], float x, float y, float z );
