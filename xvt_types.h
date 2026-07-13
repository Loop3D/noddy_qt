/* ============================================================================
 * XVT -> Qt COMPATIBILITY LAYER  (flagged file: see qt_compat/README.md)
 *
 * This header defines the historical XVT Portability Toolkit (XVT/XM, the
 * commercial GUI toolkit this application was originally written against)
 * type system: WINDOW, EVENT, RCT, PNT, COLOR, FILE_SPEC, XVT_CONFIG, and
 * the E_xxx / WC_xxx / ATTR_xxx constant families. Field layouts here are taken
 * verbatim from Providence Software Solutions' "XVT Portability Toolkit
 * Guide" (2009 ed.) and cross-checked against every call site in this
 * codebase, NOT reinvented -- so existing application code that reads or
 * writes these structures (e.g. `ep->v.mouse.where.h`, `ep->v.cmd.tag`)
 * keeps compiling and behaving identically.
 *
 * The one deliberate behavioural difference: real XVT's `v` member is a
 * C union (only one sub-struct is meaningful per event type). Here it is a
 * plain struct containing all of them. This costs a little memory per
 * EVENT and nothing else -- every existing `ep->v.xxx.yyy` access still
 * compiles and reads the value that was actually written for that event,
 * since each event constructor (in xvt_compat.cpp) only ever fills in the
 * member(s) matching its own type. This is the only place the port
 * deviates from the literal historical layout, and it is calculation-free.
 *
 * No numerical/geological/geophysical logic lives in this file or anywhere
 * in qt_compat/ -- it is pure GUI-toolkit plumbing.
 * ============================================================================
 */
#ifndef XVT_QT_COMPAT_TYPES_H
#define XVT_QT_COMPAT_TYPES_H

#include "xvt_env.h"
#include <stddef.h>
#include <stdio.h>  /* app headers (dxf.h, fcnProto.h, ...) use FILE* without including it themselves */
#include <stdlib.h> /* ditto for exit(), malloc-family, etc. */
#include <string.h> /* ditto for strcpy() etc. used alongside xvt_str_copy */
#include <limits.h> /* ditto for UCHAR_MAX etc. */
#include <strings.h> /* for strcasecmp, backing the stricmp() shim below */
#include <stdint.h>  /* for intptr_t, used by PTR_LONG() below */

/* Windows CRT case-insensitive string compare, used by a couple of the DXF
 * import files; POSIX spells this strcasecmp. */
#define stricmp strcasecmp

/* Casts a pointer to the intptr_t xvt_vobj_set_data expects, round-trippable
 * back to a pointer with the reverse cast at the read site.
 * [Qt port fix, native Windows build] Originally cast through `long`
 * (`(long)(intptr_t)(p)`), which is only pointer-width on Linux/Mac (LP64)
 * -- on native 64-bit Windows (LLP64, `long` stays 32 bits even though
 * pointers are 64) that silently truncated any heap pointer whose value
 * didn't fit in 32 bits. Confirmed as a REAL, ~100%-reproducible crash
 * (access violation), not just a theoretical risk: builder.c's
 * setupWindow() -- called for every window's very first WINDOW_INFO setup,
 * including the main History window at app startup -- did
 * `xvt_vobj_set_data(win, PTR_LONG(wip))` then immediately called
 * setCurrentObject(), whose `get_win_info(win)->tool = ...` dereferenced
 * the truncated-then-reconstructed pointer and crashed on essentially
 * every native Windows launch (confirmed via WER crash dumps + addr2line
 * against the built noddy.exe, 100% reproducible at the same fault
 * offset). This is why plain-C double-clicking noddy.exe silently
 * crashed with no window ever appearing. Fixed by widening the whole
 * chain to intptr_t: this macro, XvtObj::userData, xvt_vobj_get_data/
 * set_data's signatures (qt_compat/xvt.h + xvt_compat.cpp), and
 * xvt_win_create/xvt_win_create_def/xvt_dlg_create_res/makeWindow's
 * user_data parameter, PLUS every application-side `createCenteredWindow
 * (..., (long) somePointer)`-style call site (builder.c's own
 * createCenteredWindow/createCenteredDialog wrappers included) -- a
 * narrower, but still real-pointer-carrying, sibling of this exact
 * pattern. PICTURE (xvt_cb_get_data, clipboard) has the identical
 * long-as-handle shape but was NOT widened -- clipboard paste isn't
 * confirmed to crash (unlike this one), and is lower-risk in practice
 * (fixed small/early allocation), so left as a known, documented,
 * narrower-scope risk rather than growing this fix further. */
#define PTR_LONG(p) ((intptr_t)(p))

#ifdef __cplusplus
extern "C" {
#endif

/* ---- calling convention / prototype macros -------------------------------
 * Real XVT let apps target pre-ANSI compilers via XVT_CC_PROTO; every
 * modern compiler takes the prototyped branch. XVT_CALLCONV1/2 decorated
 * calling convention (e.g. __stdcall) on 16-bit Windows; not needed here.
 */
#define XVT_CC_PROTO 1
#define XVT_CC_ARGS(x) x
#define XVT_CALLCONV1
#define XVT_CALLCONV2

/* Alternate (older) prototype-assembly macro family used by a handful of
 * files (e.g. graph.c): `fn XVT_CC_ARGL((a,b)) XVT_CC_ARG(T1,a)
 * XVT_CC_LARG(T2,b) { ... }` assembles to `fn (T1 a, T2 b) { ... }` in
 * prototype mode (the only mode this port supports). */
#define XVT_CC_ARGL(x) (
#define XVT_CC_ARG(t, n) t n,
#define XVT_CC_LARG(t, n) t n)

/* CORRECT_WIN_RESIZE: a per-platform window-manager decoration-offset
 * workaround (defined conditionally in this app's own nodStruc.h for
 * MACWS/XOLWS/MTFWS/WINWS/WIN32WS -- none of which match our XVTWS, so the
 * app's own definition never fires). Qt's setGeometry already accounts for
 * frame decorations correctly, so this is a no-op here. */
#define CORRECT_WIN_RESIZE(W, P)

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
typedef int BOOLEAN;

typedef void *DATA_PTR;

/* ---- WINDOW ---------------------------------------------------------------
 * Real XVT: `typedef long WINDOW;` -- an opaque handle, NOT a pointer.
 * Qt backend: the "long" is an index into a handle table owned by
 * xvt_compat.cpp that maps each handle to a live QWidget*. Keeping WINDOW
 * as `long` (not QWidget*) means every existing struct field, array, and
 * cast in the application (WINDOW win; (WINDOW) xvt_vobj_get_data(...), etc)
 * keeps compiling unchanged.
 */
typedef long WINDOW;
#define NULL_WIN ((WINDOW)0)

/* Sentinel "well-known" windows; real values assigned by xvt_compat.cpp at
 * app startup (TASK_WIN is created there; SCREEN_WIN wraps the desktop). */
extern WINDOW TASK_WIN;
extern WINDOW SCREEN_WIN;

/* ---- XVT_PIXMAP ------------------------------------------------------------
 * Real XVT: a distinct handle type for pixmaps, same underlying handle
 * space as WINDOW in this port (xvt_pmap_create returns one via the same
 * allocHandle() table as windows/controls).
 */
typedef WINDOW XVT_PIXMAP;

/* ---- PNT / RCT -------------------------------------------------------------
 * Field order matches XVT exactly: PNT is (v, h) -- vertical then
 * horizontal -- and RCT is (top, left, bottom, right). Existing code that
 * positionally initializes these (`{0,0,100,100}`) or accesses `.h`/`.v`
 * keeps working identically.
 */
typedef struct {
    short v;
    short h;
} PNT;

typedef struct {
    short top;
    short left;
    short bottom;
    short right;
} RCT;

/* ---- COLOR -----------------------------------------------------------------
 * Real XVT COLOR is an opaque long-sized handle produced by
 * XVT_MAKE_COLOR(r,g,b); we keep the same packed-RGB encoding so existing
 * arithmetic/comparisons against COLOR_BLACK/COLOR_WHITE etc. still work.
 */
typedef long COLOR;
#define XVT_MAKE_COLOR(r,g,b) \
    ((COLOR)(((long)(unsigned char)(r) << 16) | \
             ((long)(unsigned char)(g) << 8)  | \
              (long)(unsigned char)(b)))
#define XVT_GET_RED(c)   ((unsigned char)(((long)(c) >> 16) & 0xff))
#define XVT_GET_GREEN(c) ((unsigned char)(((long)(c) >> 8) & 0xff))
#define XVT_GET_BLUE(c)  ((unsigned char)((long)(c) & 0xff))
/* Some app files use this "COLOR_GET" name order instead */
#define XVT_COLOR_GET_RED(c)   XVT_GET_RED(c)
#define XVT_COLOR_GET_GREEN(c) XVT_GET_GREEN(c)
#define XVT_COLOR_GET_BLUE(c)  XVT_GET_BLUE(c)

#define COLOR_BLACK   ((COLOR)0x000000L)
#define COLOR_WHITE   ((COLOR)0xffffffL)
#define COLOR_RED     ((COLOR)0xff0000L)
#define COLOR_GREEN   ((COLOR)0x00ff00L)
#define COLOR_BLUE    ((COLOR)0x0000ffL)
#define COLOR_YELLOW  ((COLOR)0xffff00L)
#define COLOR_GRAY    ((COLOR)0x808080L)
#define COLOR_LTGRAY  ((COLOR)0xc0c0c0L)
#define COLOR_DKGRAY  ((COLOR)0x404040L)

/* ---- Files / directories ---------------------------------------------------
 * Field order matches XVT exactly (dir, type, name, creator). DIRECTORY is
 * documented as opaque in the real toolkit ("you must not assume [it] is
 * even a character string") so representing it as a plain path buffer is a
 * legitimate concretization, not a deviation from any observable behavior.
 */
#define SZ_FNAME 512
typedef struct {
    char path[1024];
} DIRECTORY;

typedef struct {
    DIRECTORY dir;
    char type[6];
    char name[SZ_FNAME + 1];
    char creator[6];
} FILE_SPEC;

/* ---- XVT_CONFIG ------------------------------------------------------------
 * Matches XVT Portability Toolkit Guide sec. 2.3 exactly. */
typedef struct {
    short menu_bar_ID;
    short about_box_ID;
    char *base_appl_name;
    char *appl_name;
    char *taskwin_title;
} XVT_CONFIG;

/* ---- WIN_TYPE (control/window kinds) --------------------------------------
 * Only the members this codebase actually references are included; extend
 * as needed if a not-yet-ported file needs another WC_xxx / W_xxx constant.
 */
typedef enum {
    W_NONE, /* WIN_DEF array terminator */
    W_DOC, W_PLAIN, W_DBL, W_MODAL, W_NO_BORDER, W_PIXMAP, W_PRINT, W_SCREEN,
    WD_MODAL, WD_MODELESS,
    WC_PUSHBUTTON, WC_CHECKBOX, WC_RADIOBUTTON, WC_TEXT, WC_EDIT, WC_LBOX,
    WC_HSCROLL, WC_VSCROLL, WC_LISTBUTTON, WC_LISTEDIT, WC_GROUPBOX, WC_ICON,
    WC_TREEVIEW
} WIN_TYPE;

typedef enum { U_PIXELS, U_CHARS, U_DIALOG } UNIT_TYPE;

/* WIN_DEF: matches XVT Portability Toolkit Guide sec. 3.3.2 ("Structure-
 * based GUI Objects") verbatim -- the format `xvt_res_get_win_def` returns
 * and `xvt_win_create_def`/`xvt_dlg_create_def`/`xvt_ctl_create_def` consume.
 * An array of these describes a window/dialog (element 0) and its controls
 * (elements 1..N-1), terminated by an element with wtype == W_NONE. See
 * qt_compat/README.md for why `xvt_res_get_win_def` can't yet return the
 * *real* layout data for this app's dialogs (the resource file with that
 * data isn't in this repo) -- the struct shape itself is exact, though. */
typedef struct s_win_def {
    WIN_TYPE wtype;
    RCT rct;
    char *text;
    UNIT_TYPE units;
    DATA_PTR ctlcolors;
    union {
        struct { short menu_rid; DATA_PTR menu_p; long flags; long ctl_font_id; } win;
        struct { long flags; long ctl_font_id; } dlg;
        struct { short ctrl_id; short icon_id; long flags; long font_id; } ctl;
        struct { unsigned short attrib; long font_id; short margin; short limit; short tx_id; } tx;
    } v;
    /* Not part of real XVT's WIN_DEF -- carries the resource ID through
     * from xvt_res_get_win_def()/xvt_res_get_dlg_def() to
     * xvt_win_create_def()/xvt_dlg_create_def(), so the created window can
     * look itself up in qt_compat/dialog_registry.cpp's control-type table
     * on demand. See qt_compat/README.md. */
    long resId;
} WIN_DEF;

/* ---- Scrollbar axis selector & scroll-action constants --------------------
 * Real XVT: HSCROLL/VSCROLL identify an axis; SCROLL_CONTROL (the `what`
 * member of s_scroll) says what the user did to it. HVSCROLL is used at
 * some call sites as a single "the axis this control already represents"
 * shorthand -- keep it distinct so xvt_sbar_get_pos can special-case it.
 */
typedef enum { HSCROLL, VSCROLL, HVSCROLL } SCROLL_TYPE; /* matches parXVT.h's real (MPL-only) definition */
typedef enum {
    SC_LINE_UP, SC_LINE_DOWN, SC_PAGE_UP, SC_PAGE_DOWN, SC_THUMB, SC_THUMBTRACK
} SCROLL_CONTROL;

/* ---- EVENT -----------------------------------------------------------------
 * EVENT_TYPE matches XVT Portability Toolkit Guide sec. 4.2 verbatim.
 */
typedef enum _event_type {
    E_CREATE, E_DESTROY, E_FOCUS, E_SIZE, E_UPDATE, E_CLOSE,
    E_MOUSE_DOWN, E_MOUSE_UP, E_MOUSE_MOVE, E_MOUSE_DBL, E_MOUSE_SCROLL,
    E_CHAR, E_VSCROLL, E_HSCROLL, E_COMMAND, E_FONT, E_CONTROL, E_TIMER,
    E_QUIT, E_HELP, E_USER
} EVENT_TYPE;

typedef long MENU_TAG;

typedef struct { PNT where; int button; BOOLEAN shift, control; } MOUSE_EVENT;
/* `struct s_mouse` (forward-declared only in real XVT/this app's own headers
 * as an opaque `struct s_mouse *` parameter type) -- given the same layout
 * as MOUSE_EVENT here since that's the only shape any code actually
 * dereferences (`mouse->where`). */
struct s_mouse { PNT where; int button; BOOLEAN shift, control; };
typedef struct { char ch; int virtual_key; BOOLEAN shift, control; } CHAR_EVENT;
typedef struct { MENU_TAG tag; BOOLEAN shift, control; } CMD_EVENT;
typedef struct { SCROLL_CONTROL what; short pos; } SCROLL_EVENT;
typedef struct { long id; DATA_PTR data; } USER_EVENT;

/* CONTROL_INFO matches XVT Portability Toolkit Guide sec. 8.2 verbatim,
 * INCLUDING its `v` union wrapper (`ci.v.scroll.what`, `ci.v.edit.active`,
 * etc.) -- earlier draft of this port flattened that away by mistake,
 * which broke every one of the ~800 `ci.v.xxx` call sites in this codebase. */
typedef struct s_ctlinfo {
    WIN_TYPE type;
    WINDOW win;
    union {
        struct { BOOLEAN dbl_click; } lbox;
        struct { SCROLL_CONTROL what; short pos; } scroll;
        struct { BOOLEAN focus_change; BOOLEAN active; } edit;
        struct { int reserved; } pushbutton, radiobutton, checkbox, statictext, listbutton, groupbox, icon;
        struct { BOOLEAN focus_change; BOOLEAN active; } textedit; /* older name for the same shape as .edit */
    } v;
} CONTROL_INFO;

typedef struct { short id; CONTROL_INFO ci; } CTL_EVENT;

typedef struct s_event {
    EVENT_TYPE type;
    struct {
        BOOLEAN active;    /* E_FOCUS */
        BOOLEAN query;     /* E_QUIT */
        CMD_EVENT cmd;     /* E_COMMAND */
        CTL_EVENT ctl;     /* E_CONTROL */
        MOUSE_EVENT mouse; /* E_MOUSE_* */
        SCROLL_EVENT scroll; /* E_HSCROLL / E_VSCROLL */
        CHAR_EVENT chr;    /* E_CHAR */
        USER_EVENT user;   /* E_USER */
        struct { DATA_PTR id; } timer; /* E_TIMER */
    } v;
} EVENT;

/* WIN_EVENT_HANDLER: the `long fn(WINDOW, EVENT*)` callback shape used by
 * every one of this app's ~150 `..._eh` window/dialog handler functions.
 * Real XVT calls this type EVENT_HANDLER; some application prototypes
 * (fcnProto.h) use that name directly, so both names must exist. */
typedef long (XVT_CALLCONV1 *WIN_EVENT_HANDLER)(WINDOW, EVENT *);
typedef WIN_EVENT_HANDLER EVENT_HANDLER;

/* ---- __huge / _huge ---------------------------------------------------------
 * 16-bit-Windows/Mac "huge pointer" memory-model qualifier (both spellings
 * appear in this codebase); meaningless on any modern flat-address-space
 * platform, so it compiles away to nothing. */
#define __huge
#define _huge

/* ---- XVT_IMAGE ---------------------------------------------------------------
 * Real XVT: an opaque handle to an in-memory bitmap (distinct from
 * XVT_PIXMAP, which is a drawable window-system surface). Represented here
 * as a generic pointer, consistent with the rest of this port's [STUB]
 * image functions (see xvt.h).
 */
typedef DATA_PTR XVT_IMAGE;
enum { XVT_IMAGE_CL1, XVT_IMAGE_CL4, XVT_IMAGE_CL8, XVT_IMAGE_RGB, XVT_IMAGE_RGBA };

/* ---- Window creation: style flags (xvt_win_create's `style` bitmask) and
 * event masks (the `mask`/EVENT_MASK argument controlling which event types
 * a window receives) -------------------------------------------------------- */
typedef unsigned long EVENT_MASK;
#define EM_ALL ((EVENT_MASK)~0UL)

#define WSF_DECORATED   0x0001UL
#define WSF_ICONIZABLE  0x0002UL
#define WSF_CLOSE       0x0004UL
#define WSF_SIZE        0x0008UL
#define WSF_HSCROLL     0x0010UL
#define WSF_VSCROLL     0x0020UL
#define WSF_INVISIBLE   0x0040UL
#define WSF_NO_MENUBAR  0x0080UL

/* ---- Dialog Manager response codes (xvt_dm_post_ask etc.) ------------------ */
enum { RESP_DEFAULT = 0, RESP_2 = 1, RESP_3 = 2 };

/* ---- File-dialog status (xvt_dm_post_file_open/save) ----------------------- */
typedef enum { FL_OK, FL_CANCEL, FL_BAD } FL_STATUS;

/* ---- Help menu tag range (checked as `tag >= M_HELP && tag <= M_HELP+12`
 * in this app's task event handler); picked well above nodInc.h's own
 * resource-generated menu tags (which top out under 1200) so there's no
 * collision. ------------------------------------------------------------- */
#define M_HELP      9000
#define M_HELP_LAST (M_HELP + 12)

/* ---- Drawing tools: pens, brushes, draw mode -------------------------------
 * `DRAW_CTOOLS` (with `.pen`/`.brush`/`.mode` members) is used directly by
 * name throughout this codebase (`DRAW_CTOOLS tools;`), so unlike most other
 * types here it isn't a compat-layer invention -- just reconstructed from
 * usage since the real definition wasn't in either reference PDF fetched
 * for this port. */
typedef enum {
    M_COPY, M_OR, M_XOR, M_CLEAR, M_NOT_COPY, M_NOT_OR, M_NOT_XOR, M_NOT_CLEAR
} DRAW_MODE;

#define PAT_SOLID  0
#define PAT_HOLLOW 1
#define PAT_RUBBER 2
#define PAT_NONE   3
#define P_SOLID    PAT_SOLID
#define P_DOT      4

/* "TL_" (tool) pen/brush presets some app code assigns directly instead of
 * building a CPEN/CBRUSH -- treated as plain color+style shorthands here. */
#define TL_PEN_BLACK   COLOR_BLACK
#define TL_PEN_RUBBER  PAT_RUBBER
#define TL_BRUSH_WHITE COLOR_WHITE
#define TL_BRUSH_BLACK COLOR_BLACK

/* Some app files write `.style`, others `.pat` (both meaning the same
 * "pen pattern" concept, e.g. PAT_SOLID/PAT_HOLLOW) -- both kept as
 * distinct fields since nothing here needs to unify them. */
typedef struct { COLOR color; short width; short style; short pat; } CPEN;
typedef struct { COLOR color; short pat; } CBRUSH;
typedef struct {
    CPEN pen;
    CBRUSH brush;
    DRAW_MODE mode;
    COLOR fore_color;
    COLOR back_color;
    BOOLEAN opaque_text;
} DRAW_CTOOLS;

#define XVT_PIXMAP_DEFAULT 0
#define NULL_PIXMAP ((XVT_PIXMAP)0)
#define COLOR_INVALID ((COLOR)-1L)
#define COLOR_MAGENTA ((COLOR)0xff00ffL)
#define COLOR_CYAN    ((COLOR)0x00ffffL)

#define CURSOR_ARROW 0
#define CURSOR_CROSS 1
#define CURSOR_WAIT  2
typedef short CURSOR;

/* ---- Misc small opaque handle types (real XVT types, matching this app's
 * own MasPar-parallel-build stand-in header parXVT.h exactly) -------------- */
/* [Qt port fix] long -> intptr_t: unlike WINDOW (a small sequential handle,
 * safe to keep at `long`), PICTURE here carries a REAL heap pointer
 * (g_clipboardData in qt_compat/xvt_compat.cpp) round-tripped through
 * xvt_cb_get_data/nodLib1.c's getObjectsFromClipboard. On LLP64 Windows
 * `long` is 32 bits, so the pointer was silently truncated -- confirmed via
 * WER+addr2line crashing in getObjectsFromClipboard (nodLib1.c:1210)
 * dereferencing the truncated pointer on every Paste/Duplicate. Same root
 * cause and fix as PTR_LONG below, just for the clipboard's own opaque
 * handle instead of window/dialog user data. */
typedef intptr_t PICTURE;     /* encapsulated picture/clipboard descriptor */
typedef short TXEDIT;         /* identifies a text-edit control */
typedef unsigned short T_PNUM; /* text-edit paragraph number */
typedef DATA_PTR XVT_IOSTREAM; /* opaque I/O stream handle */
typedef struct { int placeholder; } PRINT_RCD; /* opaque print-record handle */

/* ---- Custom Control Manager (xvtcm) control-info; only referenced by the
 * small amount of custom-control glue in graph.h/profile.c. Same shape as
 * CONTROL_INFO since it plays the same "first member of my struct" role. */
typedef CONTROL_INFO XVTCM_CONTROL_INFO;

/* ---- Clipboard format tag ("Application" format, i.e. Noddy's own data,
 * as opposed to bitmap/text formats) ---------------------------------------- */
#define CB_APPL 100

#define XVT_FS_NONE   0x00
#define XVT_FS_BOLD   0x01
#define XVT_FS_ITALIC 0x02

#define CTL_FLAG_CHECKED   0x0001L
#define CTL_FLAG_INVISIBLE 0x0002L
#define CTL_FLAG_DEFAULT   0x0004L

/* ---- Dialog-response variable type (some app code declares a bare
 * `ASK_RESPONSE response;` local rather than using the RESP_* enum inline) */
typedef int ASK_RESPONSE;

/* ---- MENU_ITEM: the node type for an in-memory menu tree (built by the
 * app and passed to xvt_menu_set_tree / stored via WIN_DEF.v.win.menu_p). */
typedef struct s_menu_item {
    MENU_TAG tag;
    char *text;
    struct s_menu_item *child;
    BOOLEAN checkable;
    BOOLEAN checked;
    BOOLEAN enabled;
    int mkey;
} MENU_ITEM;

/* ---- Custom Control Manager creation-time argument block (xvtcm.h) -------
 * Passed to an app-supplied "set_creation_args" callback just before a
 * custom control is created, letting it tweak creation parameters. Only
 * graph.c in this codebase uses the Custom Control Manager. */
typedef struct { int placeholder; } WIN_CREATION_ARGS;

/* NOREF: marks a parameter as intentionally unused (XVT-Design-generated
 * code convention). */
#define NOREF(x) ((void)(x))

/* ---- far ---------------------------------------------------------------
 * 16-bit-Windows segmented-memory pointer qualifier; meaningless here. */
#define far

/* ---- ATTR_* (subset actually referenced in this codebase) ------------------ */
typedef enum {
    ATTR_SCREEN_WIDTH, ATTR_SCREEN_HEIGHT, ATTR_TITLE_HEIGHT,
    ATTR_DOCFRAME_WIDTH, ATTR_DOCFRAME_HEIGHT, ATTR_FRAME_WIDTH,
    ATTR_FRAME_HEIGHT, ATTR_BACK_COLOR, ATTR_HELP_HOOK, ATTR_MEMORY_MANAGER,
    ATTR_MAC_NO_SELECT_WINDOW, ATTR_NATIVE_WINDOW, ATTR_R40_TXEDIT_BEHAVIOR,
    ATTR_SUPPRESS_UPDATE_CHECK, ATTR_WIN_PM_NO_TWIN, ATTR_X_DISPLAY,
    ATTR_X_PLACE_WINDOW_EXACT, ATTR_X_WIDGET, ATTR_DOC_STAGGER_HORZ,
    ATTR_DOC_STAGGER_VERT
} XVT_ATTR;

#ifdef __cplusplus
}
#endif

#endif /* XVT_QT_COMPAT_TYPES_H */
