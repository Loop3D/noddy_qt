/* ============================================================================
 * [XVT -> Qt PORT] This file is otherwise unchanged calculation/application
 * logic; its XVT toolkit calls now run against the Qt5-backed compatibility
 * layer in qt_compat/ (see qt_compat/xvt.h and CLAUDE.md) instead of the
 * original proprietary XVT SDK.
 *
 * ONE deliberate exception, flagged here: removed this file's own
 * `#define TASK_MENUBAR 100` (a few lines down). It's a leftover from an
 * old, hand-maintained 100/200-based resource-ID scheme in THIS file
 * (alongside RID_ATTRIB=100, WIN_MENUBAR=200, RID_INSTR=200) that predates
 * nodInc.h's auto-generated real resource numbering (nodInc.h defines the
 * SAME symbol as 1000, guarded by `#ifndef TASK_MENUBAR` -- matching the
 * real "100 MENU" resource id confirmed from the real noddy.rc found this
 * project, see qt_compat/menu_registry.cpp's provenance). Because
 * fcnProto.h (included by nodInc.h, BEFORE nodInc.h's own guarded define)
 * includes THIS file first, builder.h's 100 was silently winning over
 * nodInc.h's intended 1000 for every translation unit that includes
 * nodInc.h (i.e. essentially the whole app) -- confirmed via gdb tracing
 * that noddy.c's `xvt_win_create(..., TASK_MENUBAR, TASK_WIN, ...)` (the
 * call that gives the main "History" window its menu bar) was receiving
 * 100, not 1000, so this port's menu_res_id==1000 check (see
 * xvt_compat.cpp's xvt_win_create) never matched and no menu bar was ever
 * built -- a real pre-existing bug in the original headers, not something
 * this port introduced, just newly load-bearing now that a real menu
 * bar exists to build. Nothing else in this codebase references the
 * literal value 100 for TASK_MENUBAR (only ever the symbolic name), so
 * removing this file's competing definition is safe. Not a calculation
 * change.
 * ============================================================================
 */
#ifndef BUILDER_INCLUDE
#define BUILDER_INCLUDE
/*
** BUILDER.H   header for builder.
*/

#include "nodStruc.h"

#ifndef MIN
#define MIN(a,b)    (((a) < (b)) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a,b)    (((a) > (b)) ? (a) : (b))
#endif


#define RID_ATTRIB      100
#define CID_TEXT     4
#define CID_OVAL     5
#define CID_RECT     6

/* TASK_MENUBAR removed from here -- see banner comment at the top of this
 * file. nodInc.h defines the real value (1000), guarded by #ifndef so it
 * only takes effect when nothing else (like this file, previously) has
 * already defined it. */
#define WIN_MENUBAR     200
#define M_HISTORY    768
#define M_TOOL_POINTER     769

#define M_HISTORY_STRATIGRAPHY   780
#define M_HISTORY_FOLD     781
#define M_HISTORY_FAULT    782
#define M_HISTORY_UNCONFORMITY   783
#define M_HISTORY_SHEAR    784
#define M_HISTORY_DYKE     785
#define M_HISTORY_PLUG     786
#define M_HISTORY_STRAIN   787
#define M_HISTORY_TILT     788
#define M_HISTORY_FOLIATION   789
#define M_HISTORY_LINEATION   790
#define M_HISTORY_STOP     791
#define M_HISTORY_IMPORT   792
#define M_HISTORY_GENERIC  793

#define RID_INSTR          200
#define LAST_RID_INSTR     (RID_INSTR + 44)

#define POINTER_ICON       10000
#define STRATIGRAPHY_ICON  10001
#define FOLD_ICON          10002
#define FAULT_ICON         10003
#define UNCONFORMITY_ICON  10004
#define SHEAR_ICON         10005
#define DYKE_ICON          10006
#define PLUG_ICON          10007
#define STRAIN_ICON        10008
#define TILT_ICON          10009
#define FOLIATION_ICON     10010
#define LINEATION_ICON     10011
#define STOP_ICON          10012
#define IMPORT_ICON        10013
#define GENERIC_ICON       10014


#define EVENT_POSITION_ICON   10100

#define FORM_ICON          10200
#define POSITION_ICON      10201
#define ORIENTATION_ICON   10202
#define SCALE_ICON         10203
#define TIME_ICON          10204
#define SURFACE_ICON       10205
#define ELLIPSOID_ICON     10206

#define STATUS_BAR         10999
/*
** General variable definitions for layout of icons
*/
#if (XVTWS == MACWS)
#define ICON_SIZE               32  /* the icon size used for events */
#define GRID_WIDTH              60  /* width of the invisable grid */
#define GRID_HEIGHT             75  /* height of the invisable grid */
#endif
#if (XVTWS == XOLWS)
#define ICON_SIZE               48  /* the icon size used for events */
#define GRID_WIDTH              80  /* width of the invisable grid */
#define GRID_HEIGHT             100 /* height of the invisable grid */
#endif
#if (XVTWS == WINWS) || (XVTWS == WIN32WS) || (XVTWS == WIN16WIN)
#define ICON_SIZE               32  /* the icon size used for events */
#define GRID_WIDTH              80  /* width of the invisable grid */
#define GRID_HEIGHT             100 /* height of the invisable grid */
#endif
#if (XVTWS == MTFWS)
#define ICON_SIZE               48  /* the icon size used for events */
#define GRID_WIDTH              80  /* width of the invisable grid */
#define GRID_HEIGHT             100 /* height of the invisable grid */
#endif
/*
**      Following typedefs are for object's shape and drawing tool.
*/
typedef enum {TL_POINTER, TL_STRATIGRAPHY,
                TL_FOLD, TL_FAULT, TL_UNCONFORMITY, TL_SHEAR, TL_DYKE,
                TL_PLUG, TL_STRAIN, TL_TILT, TL_FOLIATION, TL_LINEATION,
                TL_IMPORT, TL_STOP, TL_GENERIC} TOOL;
typedef enum { ALL_VALID, NO_STRATIGRAPHY } MENU_STATE;
typedef enum {NO_SETUP, NO_INSTRUCTS, INSTRUCTS} WIN_DATA_STATE;
/*
**      Data structure to hold information about each window.
*/
typedef struct {                        /* data for window */
        PNT origin;                     /* window upper left corner */
        PNT range;                              /* window width, height */
        OBJECT *head;                   /* object list */
        TOOL tool;                              /* current tool */
} WINDOW_INFO;


#endif
