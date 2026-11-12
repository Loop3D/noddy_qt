/* ============================================================================
 * XVT HEADLESS (NO-QT) BACKEND -- built only by `make HEADLESS=1`
 *
 * A pure-C stand-in for qt_compat/xvt_compat.cpp (and its sibling .cpp
 * files) that implements the same XVT API declared in xvt.h / statbar.h
 * WITHOUT Qt, so Noddy's batch / command-line modes (-block, -anom,
 * -random, -dxfface, ...) can be built and run on machines where Qt5 is
 * unavailable (e.g. HPC clusters).
 *
 * What is real here and what is not:
 *   - Non-GUI helpers the calculation code relies on are implemented for
 *     real, matching xvt_compat.cpp's behaviour: memory, SLIST string
 *     lists, rect arithmetic, file system, palettes, in-memory images.
 *   - Windows / dialogs / controls / pixmaps get lightweight handles in a
 *     small table (so user data, titles, rect sizes, check state round-
 *     trip) but nothing is ever shown or drawn.
 *   - Drawing, menus, help, timers, printing, clipboard: no-ops.
 *   - Message dialogs (xvt_dm_post_*) print to stderr; questions answer
 *     with the first (default) button; file dialogs report "cancelled".
 *   - xvt_app_create (the interactive GUI entry point) prints a message
 *     and the batch usage, then exits: there is no GUI in this build.
 *
 * Kept in its own sub-directory so the normal Qt build's
 * wildcard of top-level qt_compat C files never picks it up.
 * ============================================================================
 */
#include "xvt.h"
#include "statbar.h"

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#include <io.h>
#define getcwd _getcwd
#define chdir  _chdir
#define access _access
#ifndef R_OK
#define R_OK 4
#endif
#else
#include <unistd.h>
#endif

#define HEADLESS_SCREEN_WIDTH  1920
#define HEADLESS_SCREEN_HEIGHT 1080

/* Same as the Qt build: in batch mode there is no task window. */
WINDOW TASK_WIN = NULL_WIN;
WINDOW SCREEN_WIN = NULL_WIN;

/* ==========================================================================
 * Handle table: WINDOW / XVT_PIXMAP values are small integers indexing it
 * ========================================================================== */
typedef struct {
    int inUse;
    WIN_TYPE type;
    WINDOW parent;
    RCT rct;            /* client size (pixmaps: their pixel size) */
    intptr_t userData;
    WIN_EVENT_HANDLER eh;
    char *title;
    BOOLEAN enabled, visible, checked;
    int sbarPos, sbarLo, sbarHi;
    COLOR backColor;
} HeadlessObj;

static HeadlessObj *g_objs = NULL;
static long g_numObjs = 0; /* slot 0 is never used: it is NULL_WIN */

static HeadlessObj *objFor(WINDOW win)
{
    if (win <= 0 || win >= g_numObjs || !g_objs[win].inUse) return NULL;
    return &g_objs[win];
}

static WINDOW allocHandle(WIN_TYPE type, const RCT *rct, const char *title,
                          WINDOW parent, WIN_EVENT_HANDLER eh, intptr_t userData)
{
    long h;
    HeadlessObj *o;

    for (h = 1; h < g_numObjs; h++)
        if (!g_objs[h].inUse) break;
    if (h >= g_numObjs) {
        long newNum = g_numObjs ? g_numObjs * 2 : 64;
        HeadlessObj *grown = (HeadlessObj *)realloc(g_objs, (size_t)newNum * sizeof(HeadlessObj));
        if (!grown) return NULL_WIN;
        memset(grown + g_numObjs, 0, (size_t)(newNum - g_numObjs) * sizeof(HeadlessObj));
        g_objs = grown;
        g_numObjs = newNum;
    }
    o = &g_objs[h];
    memset(o, 0, sizeof(*o));
    o->inUse = TRUE;
    o->type = type;
    o->parent = parent;
    if (rct) {
        o->rct.left = 0; o->rct.top = 0;
        o->rct.right = (short)(rct->right - rct->left);
        o->rct.bottom = (short)(rct->bottom - rct->top);
    }
    o->title = title ? strdup(title) : NULL;
    o->eh = eh;
    o->userData = userData;
    o->enabled = TRUE;
    o->visible = TRUE;
    o->backColor = COLOR_WHITE;
    return (WINDOW)h;
}

static void freeHandle(WINDOW win)
{
    HeadlessObj *o = objFor(win);
    if (!o) return;
    free(o->title);
    memset(o, 0, sizeof(*o));
}

static void vreport(const char *kind, const char *fmt, va_list ap)
{
    fprintf(stderr, "\nNoddy %s: ", kind);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    fflush(stderr);
}

/* ==========================================================================
 * Application / task window
 * ========================================================================== */
extern void displayUsage(char *commandName); /* nodLib3.c */

int xvt_app_create(int argc, char **argv, long flags, WIN_EVENT_HANDLER task_eh, XVT_CONFIG *cfg)
{
    (void)argc; (void)flags; (void)task_eh; (void)cfg;
    fprintf(stderr, "\nThis Noddy was built without a GUI (make HEADLESS=1);"
                    "\nonly batch / command-line operation is available.\n");
    displayUsage(argv && argv[0] ? argv[0] : (char *)"noddy");
    exit(1);
    return 0;
}

void xvt_hide_console_if_owned(void) {}
void xvt_app_destroy(void) {}
BOOLEAN xvt_app_allow_quit(void) { return TRUE; }
void xvt_app_process_pending_events(void) {}

void xvt_app_get_default_ctools(DRAW_CTOOLS *tools)
{
    if (!tools) return;
    memset(tools, 0, sizeof(*tools));
    tools->pen.color = COLOR_BLACK;
    tools->pen.width = 1;
    tools->pen.pat = PAT_SOLID;
    tools->brush.color = COLOR_WHITE;
    tools->brush.pat = PAT_SOLID;
    tools->mode = M_COPY;
    tools->fore_color = COLOR_BLACK;
    tools->back_color = COLOR_WHITE;
}

/* ==========================================================================
 * vobj
 * ========================================================================== */
long xvt_vobj_get_attr(WINDOW win, XVT_ATTR attr)
{
    HeadlessObj *o;
    switch (attr) {
    case ATTR_SCREEN_WIDTH:  return HEADLESS_SCREEN_WIDTH;
    case ATTR_SCREEN_HEIGHT: return HEADLESS_SCREEN_HEIGHT;
    case ATTR_TITLE_HEIGHT:  return 24;
    case ATTR_DOCFRAME_WIDTH:
    case ATTR_FRAME_WIDTH:   return 4;
    case ATTR_DOCFRAME_HEIGHT:
    case ATTR_FRAME_HEIGHT:  return 4;
    case ATTR_BACK_COLOR:
        o = objFor(win);
        return o ? o->backColor : COLOR_WHITE;
    case ATTR_DOC_STAGGER_HORZ: return 24;
    case ATTR_DOC_STAGGER_VERT: return 24;
    default: return 0;
    }
}

void xvt_vobj_set_attr(WINDOW win, XVT_ATTR attr, long value)
{
    HeadlessObj *o = objFor(win);
    if (o && attr == ATTR_BACK_COLOR) o->backColor = (COLOR)value;
}

intptr_t xvt_vobj_get_data(WINDOW win)
{
    HeadlessObj *o = objFor(win);
    return o ? o->userData : 0;
}

void xvt_vobj_set_data(WINDOW win, intptr_t data)
{
    HeadlessObj *o = objFor(win);
    if (o) o->userData = data;
}

void xvt_vobj_destroy(WINDOW win) { freeHandle(win); }

void xvt_vobj_set_enabled(WINDOW win, BOOLEAN enabled)
{
    HeadlessObj *o = objFor(win);
    if (o) o->enabled = enabled;
}
BOOLEAN xvt_vobj_get_enabled(WINDOW win)
{
    HeadlessObj *o = objFor(win);
    return o ? o->enabled : FALSE;
}
void xvt_vobj_set_visible(WINDOW win, BOOLEAN visible)
{
    HeadlessObj *o = objFor(win);
    if (o) o->visible = visible;
}
BOOLEAN xvt_vobj_get_visible(WINDOW win)
{
    HeadlessObj *o = objFor(win);
    return o ? o->visible : FALSE;
}

void xvt_vobj_set_title(WINDOW win, const char *title)
{
    HeadlessObj *o = objFor(win);
    if (!o) return;
    free(o->title);
    o->title = title ? strdup(title) : NULL;
}

BOOLEAN xvt_vobj_get_title(WINDOW win, char *buf, int maxlen)
{
    HeadlessObj *o = objFor(win);
    if (!buf || maxlen <= 0) return FALSE;
    buf[0] = '\0';
    if (!o) return FALSE;
    if (o->title) {
        strncpy(buf, o->title, (size_t)maxlen - 1);
        buf[maxlen - 1] = '\0';
    }
    return TRUE;
}

void xvt_vobj_move(WINDOW win, RCT *rct)
{
    HeadlessObj *o = objFor(win);
    if (!o || !rct) return;
    o->rct.left = 0; o->rct.top = 0;
    o->rct.right = (short)(rct->right - rct->left);
    o->rct.bottom = (short)(rct->bottom - rct->top);
}

void xvt_vobj_get_client_rect(WINDOW win, RCT *rct)
{
    HeadlessObj *o = objFor(win);
    if (!rct) return;
    if (o) *rct = o->rct;
    else if (win == SCREEN_WIN)
        xvt_rect_set(rct, 0, 0, HEADLESS_SCREEN_WIDTH, HEADLESS_SCREEN_HEIGHT);
    else
        rct->left = rct->top = rct->right = rct->bottom = 0;
}

void xvt_vobj_get_outer_rect(WINDOW win, RCT *rct) { xvt_vobj_get_client_rect(win, rct); }

WINDOW xvt_vobj_get_parent(WINDOW win)
{
    HeadlessObj *o = objFor(win);
    return o ? o->parent : NULL_WIN;
}

WIN_TYPE xvt_vobj_get_type(WINDOW win)
{
    HeadlessObj *o = objFor(win);
    return o ? o->type : W_NONE;
}

void xvt_vobj_translate_points(WINDOW from, WINDOW to, PNT *pts, int n)
{
    (void)from; (void)to; (void)pts; (void)n; /* no screen geometry: identity */
}

void xvt_vobj_raise(WINDOW win) { (void)win; }
void xvt_vobj_set_palet(WINDOW win, DATA_PTR palet) { (void)win; (void)palet; }
DATA_PTR xvt_vobj_get_palet(WINDOW win) { (void)win; return NULL; }

/* ==========================================================================
 * Windows / dialogs / controls -- handles only, never displayed
 * ========================================================================== */
WINDOW xvt_win_create(WIN_TYPE type, RCT *rct, const char *title, long menu_res_id,
                      WINDOW parent, unsigned long style, EVENT_MASK mask,
                      WIN_EVENT_HANDLER eh, intptr_t user_data)
{
    (void)menu_res_id; (void)style; (void)mask;
    return allocHandle(type, rct, title, parent, eh, user_data);
}

WINDOW xvt_win_create_def(WIN_DEF *def, WINDOW parent, EVENT_MASK mask,
                          WIN_EVENT_HANDLER eh, intptr_t user_data)
{
    (void)mask;
    if (!def) return NULL_WIN;
    return allocHandle(def->wtype, &def->rct, def->text, parent, eh, user_data);
}

WINDOW xvt_win_get_ctl(WINDOW win, int ctl_id) { (void)win; (void)ctl_id; return NULL_WIN; }
void xvt_win_set_cursor(WINDOW win, int cursor_id) { (void)win; (void)cursor_id; }

long xvt_win_dispatch_event(WINDOW win, EVENT *ev)
{
    HeadlessObj *o = objFor(win);
    return (o && o->eh && ev) ? o->eh(win, ev) : 0L;
}

TXEDIT xvt_win_get_tx(WINDOW win, int ctl_id) { (void)win; (void)ctl_id; return 0; }
void xvt_win_trap_pointer(WINDOW win) { (void)win; }
void xvt_win_release_pointer(void) {}

WINDOW xvt_dlg_create_res(WIN_TYPE dlg_type, long dlg_res_id, EVENT_MASK mask,
                          WIN_EVENT_HANDLER eh, intptr_t user_data)
{
    RCT rct;
    (void)dlg_res_id; (void)mask;
    xvt_rect_set(&rct, 0, 0, 400, 300);
    return allocHandle(dlg_type, &rct, NULL, NULL_WIN, eh, user_data);
}

WINDOW xvt_dlg_create_def(WIN_DEF *def, EVENT_MASK mask,
                          WIN_EVENT_HANDLER eh, intptr_t user_data)
{
    return xvt_win_create_def(def, NULL_WIN, mask, eh, user_data);
}

void xvt_begin_long_job_ui(void) {}
void xvt_end_long_job_ui(void) {}

WINDOW xvt_ctl_create(WIN_TYPE type, RCT *rct, const char *title, WINDOW parent,
                      long flags, long font_id, int ctl_id)
{
    WINDOW w;
    HeadlessObj *o;
    (void)font_id; (void)ctl_id;
    w = allocHandle(type, rct, title, parent, NULL, 0);
    if ((o = objFor(w)) != NULL) o->checked = (flags & CTL_FLAG_CHECKED) ? TRUE : FALSE;
    return w;
}

WINDOW xvt_custom_ctl_create(int cid, RCT *rct, WINDOW parent, WIN_EVENT_HANDLER eh)
{
    (void)cid;
    return allocHandle(W_PLAIN, rct, NULL, parent, eh, 0);
}

void xvt_win_set_ctl_proxy(WINDOW win, WINDOW proxyWin) { (void)win; (void)proxyWin; }
void xvt_win_mark_controls_only(WINDOW win) { (void)win; }
void xvt_win_set_redraw_proxy(WINDOW win, WINDOW proxyWin) { (void)win; (void)proxyWin; }
int xvt_win_get_menubar_height(WINDOW win) { (void)win; return 0; }

BOOLEAN xvt_ctl_is_checked(WINDOW ctl_win)
{
    HeadlessObj *o = objFor(ctl_win);
    return o ? o->checked : FALSE;
}
void xvt_ctl_set_checked(WINDOW ctl_win, BOOLEAN checked)
{
    HeadlessObj *o = objFor(ctl_win);
    if (o) o->checked = checked;
}
void xvt_ctl_check_radio_button(WINDOW checked, WINDOW *group, int count)
{
    int i;
    for (i = 0; group && i < count; i++)
        xvt_ctl_set_checked(group[i], group[i] == checked);
}
long xvt_ctl_get_id(WINDOW ctl_win) { (void)ctl_win; return 0; }

/* ---- list boxes: always empty ---- */
void xvt_list_add(WINDOW list_win, int index, const char *text) { (void)list_win; (void)index; (void)text; }
void xvt_list_clear(WINDOW list_win) { (void)list_win; }
int  xvt_list_get_sel_index(WINDOW list_win) { (void)list_win; return -1; }
BOOLEAN xvt_list_get_first_sel(WINDOW list_win, char *buf, int maxlen)
{
    (void)list_win;
    if (buf && maxlen > 0) buf[0] = '\0';
    return FALSE;
}
BOOLEAN xvt_list_set_sel(WINDOW list_win, int index, BOOLEAN sel) { (void)list_win; (void)index; (void)sel; return FALSE; }
BOOLEAN xvt_list_is_sel(WINDOW list_win, int index) { (void)list_win; (void)index; return FALSE; }
int  xvt_list_count_all(WINDOW list_win) { (void)list_win; return 0; }
void xvt_list_suspend(WINDOW list_win) { (void)list_win; }
void xvt_list_resume(WINDOW list_win) { (void)list_win; }

/* ---- scroll bars: remember position/range ---- */
int xvt_sbar_get_pos(WINDOW win, SCROLL_TYPE axis)
{
    HeadlessObj *o = objFor(win);
    (void)axis;
    return o ? o->sbarPos : 0;
}
void xvt_sbar_set_pos(WINDOW win, SCROLL_TYPE axis, int pos)
{
    HeadlessObj *o = objFor(win);
    (void)axis;
    if (o) o->sbarPos = pos;
}
void xvt_sbar_set_range(WINDOW win, SCROLL_TYPE axis, int lo, int hi)
{
    HeadlessObj *o = objFor(win);
    (void)axis;
    if (o) { o->sbarLo = lo; o->sbarHi = hi; }
}
void xvt_sbar_get_range(WINDOW win, SCROLL_TYPE axis, int *lo, int *hi)
{
    HeadlessObj *o = objFor(win);
    (void)axis;
    if (lo) *lo = o ? o->sbarLo : 0;
    if (hi) *hi = o ? o->sbarHi : 0;
}

/* ---- menus ---- */
void xvt_menu_set_item_enabled(WINDOW win, MENU_TAG tag, BOOLEAN enabled) { (void)win; (void)tag; (void)enabled; }
void xvt_menu_set_item_checked(WINDOW win, MENU_TAG tag, BOOLEAN checked) { (void)win; (void)tag; (void)checked; }
void xvt_menu_set_tree(WINDOW win, DATA_PTR tree) { (void)win; (void)tree; }
DATA_PTR xvt_menu_get_tree(WINDOW win) { (void)win; return NULL; }
void xvt_menu_update(WINDOW win) { (void)win; }

/* ==========================================================================
 * Message / file dialogs -> stderr
 * ========================================================================== */
void xvt_dm_post_error(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vreport("ERROR", fmt, ap);
    va_end(ap);
}

void xvt_dm_post_note(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vreport("note", fmt, ap);
    va_end(ap);
}

int xvt_dm_post_ask(const char *b1, const char *b2, const char *b3, const char *fmt, ...)
{
    va_list ap;
    (void)b2; (void)b3;
    va_start(ap, fmt);
    vreport("question", fmt, ap);
    va_end(ap);
    /* No one to ask: take the first (default) button, the same answer
     * the Qt build gives when the question is dismissed. */
    fprintf(stderr, "  -> answering \"%s\" (no GUI)\n", b1 ? b1 : "default");
    return RESP_DEFAULT;
}

FL_STATUS xvt_dm_post_file_open(FILE_SPEC *fs, const char *prompt)
{
    (void)fs;
    fprintf(stderr, "\nNoddy: file dialog \"%s\" cancelled (no GUI)\n", prompt ? prompt : "Open");
    return FL_CANCEL;
}

FL_STATUS xvt_dm_post_file_save(FILE_SPEC *fs, const char *prompt)
{
    (void)fs;
    fprintf(stderr, "\nNoddy: file dialog \"%s\" cancelled (no GUI)\n", prompt ? prompt : "Save");
    return FL_CANCEL;
}

FL_STATUS xvt_dm_post_dir_select(DIRECTORY *dir, const char *prompt)
{
    (void)dir;
    fprintf(stderr, "\nNoddy: directory dialog \"%s\" cancelled (no GUI)\n", prompt ? prompt : "Select Directory");
    return FL_CANCEL;
}

BOOLEAN xvt_dm_post_string_prompt(const char *prompt, char *buf, int maxlen)
{
    (void)prompt; (void)buf; (void)maxlen;
    return FALSE;
}

BOOLEAN xvt_dm_post_page_setup(PRINT_RCD *printer_setup) { (void)printer_setup; return FALSE; }

/* ==========================================================================
 * Drawing -- all no-ops
 * ========================================================================== */
void xvt_dwin_clear(WINDOW win, COLOR color) { (void)win; (void)color; }
void xvt_dwin_update(WINDOW win) { (void)win; }
void xvt_dwin_invalidate_rect(WINDOW win, RCT *rct) { (void)win; (void)rct; }
BOOLEAN xvt_dwin_is_update_needed(WINDOW win, RCT *rct) { (void)win; (void)rct; return FALSE; }
void xvt_dwin_draw_set_pos(WINDOW win, PNT pos) { (void)win; (void)pos; }
void xvt_dwin_draw_line(WINDOW win, PNT pos) { (void)win; (void)pos; }
void xvt_dwin_draw_aline(WINDOW win, PNT to, BOOLEAN close, BOOLEAN visible) { (void)win; (void)to; (void)close; (void)visible; }
void xvt_dwin_draw_rect(WINDOW win, RCT *rct) { (void)win; (void)rct; }
void xvt_dwin_draw_roundrect(WINDOW win, RCT *rct, int ovalw, int ovalh) { (void)win; (void)rct; (void)ovalw; (void)ovalh; }
void xvt_dwin_draw_oval(WINDOW win, RCT *rct) { (void)win; (void)rct; }
void xvt_dwin_draw_polygon(WINDOW win, PNT *pts, int n) { (void)win; (void)pts; (void)n; }
void xvt_dwin_draw_polyline(WINDOW win, PNT *pts, int n) { (void)win; (void)pts; (void)n; }
void xvt_dwin_draw_text(WINDOW win, short x, short y, const char *text, int len) { (void)win; (void)x; (void)y; (void)text; (void)len; }
int  xvt_dwin_get_text_width(WINDOW win, const char *text, int len)
{
    (void)win;
    if (!text) return 0;
    if (len < 0) len = (int)strlen(text);
    return 7 * len; /* nominal fixed-width estimate */
}
int  xvt_dwin_text_width(WINDOW win, const char *text, int len) { return xvt_dwin_get_text_width(win, text, len); }
void xvt_dwin_draw_icon(WINDOW win, short x, short y, int icon_id) { (void)win; (void)x; (void)y; (void)icon_id; }
void xvt_dwin_draw_pmap(WINDOW win, XVT_PIXMAP pmap, RCT *dst, RCT *src) { (void)win; (void)pmap; (void)dst; (void)src; }
void xvt_dwin_draw_image(WINDOW win, XVT_IMAGE image, RCT *dst, RCT *src) { (void)win; (void)image; (void)dst; (void)src; }
void xvt_dwin_scroll_rect(WINDOW win, RCT *rct, int dh, int dv) { (void)win; (void)rct; (void)dh; (void)dv; }
void xvt_dwin_set_clip(WINDOW win, RCT *rct) { (void)win; (void)rct; }
void xvt_dwin_set_fore_color(WINDOW win, COLOR color) { (void)win; (void)color; }
void xvt_dwin_set_back_color(WINDOW win, COLOR color) { (void)win; (void)color; }
void xvt_dwin_set_font_family(WINDOW win, const char *family) { (void)win; (void)family; }
void xvt_dwin_set_font_style(WINDOW win, int style) { (void)win; (void)style; }
void xvt_dwin_set_font_size(WINDOW win, int size) { (void)win; (void)size; }
void xvt_dwin_set_font(WINDOW win, DATA_PTR font) { (void)win; (void)font; }
DATA_PTR xvt_dwin_get_font(WINDOW win) { (void)win; return NULL; }
void xvt_dwin_set_draw_mode(WINDOW win, int mode) { (void)win; (void)mode; }
void xvt_dwin_set_cpen(WINDOW win, CPEN *pen) { (void)win; (void)pen; }
void xvt_dwin_set_std_cpen(WINDOW win, int std_pen) { (void)win; (void)std_pen; }
void xvt_dwin_set_std_cbrush(WINDOW win, int std_brush) { (void)win; (void)std_brush; }
void xvt_dwin_get_font_metrics(WINDOW win, int *leading, int *ascent, int *descent)
{
    (void)win;
    if (leading) *leading = 0;
    if (ascent) *ascent = 10;
    if (descent) *descent = 3;
}
void xvt_font_get_metrics(DATA_PTR font, int *leadingp, int *ascentp, int *descentp)
{
    (void)font;
    xvt_dwin_get_font_metrics(NULL_WIN, leadingp, ascentp, descentp);
}
void xvt_dwin_set_draw_ctools(WINDOW win, DRAW_CTOOLS *tools) { (void)win; (void)tools; }
void xvt_dwin_get_draw_ctools(WINDOW win, DRAW_CTOOLS *tools) { (void)win; xvt_app_get_default_ctools(tools); }

/* ==========================================================================
 * Pixmaps (sized handles, never drawn into) and images (real pixel buffers)
 * ========================================================================== */
XVT_PIXMAP xvt_pmap_create(WINDOW win, int type, short width, short height, DATA_PTR palette)
{
    RCT rct;
    (void)win; (void)type; (void)palette;
    xvt_rect_set(&rct, 0, 0, width > 0 ? width : 1, height > 0 ? height : 1);
    return allocHandle(W_PIXMAP, &rct, NULL, NULL_WIN, NULL, 0);
}

void xvt_pmap_destroy(XVT_PIXMAP pmap) { freeHandle(pmap); }

typedef struct { short w, h; COLOR *pixels; } HeadlessImage;

XVT_IMAGE xvt_image_create(int type, short w, short h, DATA_PTR clut)
{
    HeadlessImage *img;
    long i, n;
    (void)type; (void)clut;
    if (w < 1) w = 1;
    if (h < 1) h = 1;
    if (!(img = (HeadlessImage *)malloc(sizeof(HeadlessImage)))) return NULL;
    n = (long)w * h;
    if (!(img->pixels = (COLOR *)malloc((size_t)n * sizeof(COLOR)))) { free(img); return NULL; }
    for (i = 0; i < n; i++) img->pixels[i] = COLOR_WHITE;
    img->w = w; img->h = h;
    return (XVT_IMAGE)img;
}

void xvt_image_destroy(XVT_IMAGE image)
{
    HeadlessImage *img = (HeadlessImage *)image;
    if (!img) return;
    free(img->pixels);
    free(img);
}

XVT_IMAGE xvt_image_read(const char *filename)
{
    (void)filename; /* no image decoders in the headless build */
    return NULL;
}

void xvt_image_get_dimensions(XVT_IMAGE image, short *w, short *h)
{
    HeadlessImage *img = (HeadlessImage *)image;
    if (w) *w = img ? img->w : 0;
    if (h) *h = img ? img->h : 0;
}

void xvt_image_set_pixel(XVT_IMAGE image, short x, short y, COLOR c)
{
    HeadlessImage *img = (HeadlessImage *)image;
    if (img && x >= 0 && y >= 0 && x < img->w && y < img->h)
        img->pixels[(long)y * img->w + x] = c;
}

COLOR xvt_image_get_pixel(XVT_IMAGE image, short x, short y)
{
    HeadlessImage *img = (HeadlessImage *)image;
    if (!img || x < 0 || y < 0 || x >= img->w || y >= img->h) return COLOR_BLACK;
    return img->pixels[(long)y * img->w + x];
}

void xvt_image_get_from_pmap(XVT_IMAGE image, XVT_PIXMAP pmap, RCT *dst, RCT *src)
{
    (void)image; (void)pmap; (void)dst; (void)src; /* pixmaps hold no pixels here */
}

void xvt_image_set_ncolors(XVT_IMAGE image, int n) { (void)image; (void)n; }
void xvt_image_set_clut(XVT_IMAGE image, COLOR *clut, int n) { (void)image; (void)clut; (void)n; }

BOOLEAN xvt_image_write_bmp_to_iostr(XVT_IMAGE image, DATA_PTR iostr)
{
    (void)image; (void)iostr;
    fprintf(stderr, "\nNoddy: saving pictures is not available in the headless build\n");
    return FALSE;
}

BOOLEAN xvt_image_write_macpict_to_iostr(XVT_IMAGE image, DATA_PTR iostr)
{
    return xvt_image_write_bmp_to_iostr(image, iostr);
}

/* ==========================================================================
 * Rects / geometry (same arithmetic as xvt_compat.cpp)
 * ========================================================================== */
void xvt_rect_set(RCT *r, short left, short top, short right, short bottom)
{
    if (!r) return;
    r->left = left; r->top = top; r->right = right; r->bottom = bottom;
}

void xvt_rect_set_pos(RCT *r, PNT pos)
{
    short w, h;
    if (!r) return;
    w = r->right - r->left; h = r->bottom - r->top;
    r->left = pos.h; r->top = pos.v;
    r->right = r->left + w; r->bottom = r->top + h;
}

void xvt_rect_offset(RCT *r, short dh, short dv)
{
    if (!r) return;
    r->left += dh; r->right += dh; r->top += dv; r->bottom += dv;
}

void xvt_rect_set_width(RCT *r, short w) { if (r) r->right = r->left + w; }
void xvt_rect_set_height(RCT *r, short h) { if (r) r->bottom = r->top + h; }
short xvt_rect_get_width(const RCT *r) { return r ? (short)(r->right - r->left) : 0; }
short xvt_rect_get_height(const RCT *r) { return r ? (short)(r->bottom - r->top) : 0; }

BOOLEAN xvt_rect_has_point(const RCT *r, PNT pt)
{
    if (!r) return FALSE;
    return (pt.h >= r->left && pt.h < r->right && pt.v >= r->top && pt.v < r->bottom) ? TRUE : FALSE;
}

BOOLEAN xvt_rect_intersect(RCT *dst, const RCT *a, const RCT *b)
{
    RCT r;
    if (!dst || !a || !b) return FALSE;
    r.left   = a->left   > b->left   ? a->left   : b->left;
    r.top    = a->top    > b->top    ? a->top    : b->top;
    r.right  = a->right  < b->right  ? a->right  : b->right;
    r.bottom = a->bottom < b->bottom ? a->bottom : b->bottom;
    if (r.right <= r.left || r.bottom <= r.top) {
        dst->left = dst->top = dst->right = dst->bottom = 0;
        return FALSE;
    }
    *dst = r;
    return TRUE;
}

/* ==========================================================================
 * Memory (same as xvt_compat.cpp)
 * ========================================================================== */
DATA_PTR xvt_mem_alloc(size_t size) { return malloc(size); }
DATA_PTR xvt_mem_zalloc(size_t size) { return calloc(1, size); }
DATA_PTR xvt_mem_realloc(DATA_PTR p, size_t size)
{
    if (!p) return calloc(1, size); /* see xvt_compat.cpp: growing from NULL is zero-filled */
    return realloc(p, size);
}
void xvt_mem_free(DATA_PTR p) { free(p); }
DATA_PTR xvt_mem_halloc(long count, size_t elemsize) { return calloc((size_t)count, elemsize); }
DATA_PTR xvt_mem_rep(DATA_PTR p, size_t oldsize, size_t newsize) { (void)oldsize; return realloc(p, newsize); }

/* ==========================================================================
 * SLIST (same semantics as xvt_compat.cpp: insertion order, owned text)
 * ========================================================================== */
struct s_xvt_slist_elt { long data; char *text; struct s_xvt_slist_elt *next; };
struct s_xvt_slist { struct s_xvt_slist_elt *head, *tail, *iter; long count; };

SLIST xvt_slist_create(void) { return (SLIST)calloc(1, sizeof(struct s_xvt_slist)); }

void xvt_slist_destroy(SLIST l)
{
    struct s_xvt_slist_elt *e, *next;
    if (!l) return;
    for (e = l->head; e; e = next) {
        next = e->next;
        free(e->text);
        free(e);
    }
    free(l);
}

SLIST_ELT xvt_slist_get_first(SLIST l)
{
    if (!l) return NULL;
    l->iter = l->head;
    return l->iter;
}

SLIST_ELT xvt_slist_get_next(SLIST l, SLIST_ELT elt)
{
    (void)elt;
    if (!l || !l->iter) return NULL;
    l->iter = l->iter->next;
    return l->iter;
}

char *xvt_slist_get(SLIST l, SLIST_ELT elt, long *data)
{
    (void)l;
    if (data) *data = elt ? elt->data : 0;
    return elt ? elt->text : NULL;
}

void xvt_slist_add_sorted(SLIST l, const char *name, long data, BOOLEAN unique, BOOLEAN case_sensitive)
{
    struct s_xvt_slist_elt *e;
    (void)unique; (void)case_sensitive;
    if (!l) return;
    if (!(e = (struct s_xvt_slist_elt *)calloc(1, sizeof(*e)))) return;
    e->data = data;
    e->text = strdup(name ? name : "");
    if (l->tail) l->tail->next = e; else l->head = e;
    l->tail = e;
    l->count++;
}

long xvt_slist_count(SLIST l) { return l ? l->count : 0; }

/* ==========================================================================
 * File system
 * ========================================================================== */
static void fullPath(FILE_SPEC *fs, char *buf, size_t size)
{
    if (fs->dir.path[0])
        snprintf(buf, size, "%s/%s", fs->dir.path, fs->name);
    else
        snprintf(buf, size, "%s", fs->name);
}

BOOLEAN xvt_fsys_get_default_dir(DIRECTORY *dir)
{
    if (!dir) return FALSE;
    if (!getcwd(dir->path, sizeof(dir->path))) { dir->path[0] = '\0'; return FALSE; }
    return TRUE;
}

BOOLEAN xvt_fsys_set_dir(DIRECTORY *dir)
{
    if (!dir) return FALSE;
    return chdir(dir->path) == 0 ? TRUE : FALSE;
}

BOOLEAN xvt_fsys_get_dir(DIRECTORY *dir) { return xvt_fsys_get_default_dir(dir); }

BOOLEAN xvt_fsys_set_dir_startup(void)
{
    /* Deliberately leaves the working directory alone. In the Qt build a
     * batch run never creates a QApplication, so its applicationDirPath()
     * is empty and this call does not change directory either -- relative
     * input/output paths given on the command line must keep resolving
     * against the directory noddy was run from. */
    return FALSE;
}

BOOLEAN xvt_fsys_convert_str_to_dir(const char *str, DIRECTORY *dir)
{
    if (!dir || !str) return FALSE;
    strncpy(dir->path, str, sizeof(dir->path) - 1);
    dir->path[sizeof(dir->path) - 1] = '\0';
    return TRUE;
}

BOOLEAN xvt_fsys_convert_dir_to_str(DIRECTORY *dir, char *buf, int maxlen)
{
    if (!dir || !buf || maxlen <= 0) return FALSE;
    strncpy(buf, dir->path, (size_t)maxlen - 1);
    buf[maxlen - 1] = '\0';
    return TRUE;
}

BOOLEAN xvt_fsys_get_file_attr(FILE_SPEC *fs, unsigned long attr)
{
    char path[sizeof(fs->dir.path) + SZ_FNAME + 2];
    struct stat st;
    if (!fs) return FALSE;
    fullPath(fs, path, sizeof(path));
    if (attr == XVT_FILE_ATTR_READ) return access(path, R_OK) == 0 ? TRUE : FALSE;
    if (attr == XVT_FILE_ATTR_DIRECTORY)
        return (stat(path, &st) == 0 && (st.st_mode & S_IFMT) == S_IFDIR) ? TRUE : FALSE;
    return FALSE;
}

BOOLEAN xvt_fsys_set_file_attr(FILE_SPEC *fs, unsigned long attr, long value)
{
    (void)fs; (void)attr; (void)value; /* Mac type/creator codes: no-op */
    return TRUE;
}

BOOLEAN xvt_fsys_rem_file(FILE_SPEC *fs)
{
    char path[sizeof(fs->dir.path) + SZ_FNAME + 2];
    if (!fs) return FALSE;
    fullPath(fs, path, sizeof(path));
    return remove(path) == 0 ? TRUE : FALSE;
}

/* ==========================================================================
 * Resources
 * ========================================================================== */
void xvt_xres_build_icon(ICON_RESOURCE *iconp, unsigned h, unsigned w, DATA_PTR data) { (void)iconp; (void)h; (void)w; (void)data; }
void xvt_res_free_menu_tree(DATA_PTR tree) { (void)tree; }
void xvt_res_free_win_def(WIN_DEF *def) { free(def); }

static WIN_DEF *placeholderWinDef(long resId)
{
    WIN_DEF *def = (WIN_DEF *)calloc(2, sizeof(WIN_DEF));
    if (!def) return NULL;
    def[0].wtype = W_DOC;
    def[0].resId = resId;
    xvt_rect_set(&def[0].rct, 100, 100, 500, 400);
    def[1].wtype = W_NONE;
    return def;
}

WIN_DEF *xvt_res_get_win_def(long res_id) { return placeholderWinDef(res_id); }
WIN_DEF *xvt_res_get_dlg_def(long res_id) { return placeholderWinDef(res_id); }
DATA_PTR xvt_res_get_menu(long res_id) { (void)res_id; return NULL; }

/* ==========================================================================
 * Palettes (real colour list, same as xvt_compat.cpp)
 * ========================================================================== */
typedef struct { COLOR *colors; int count, size; long tolerance; } HeadlessPalette;

XVT_PALETTE xvt_palet_create(int type, DATA_PTR reserved)
{
    (void)type; (void)reserved;
    return (XVT_PALETTE)calloc(1, sizeof(HeadlessPalette));
}

void xvt_palet_destroy(XVT_PALETTE p)
{
    HeadlessPalette *pal = (HeadlessPalette *)p;
    if (!pal) return;
    free(pal->colors);
    free(pal);
}

void xvt_palet_set_tolerance(XVT_PALETTE p, long tol)
{
    if (p) ((HeadlessPalette *)p)->tolerance = tol;
}

void xvt_palet_add_colors(XVT_PALETTE p, COLOR *colors, short n)
{
    HeadlessPalette *pal = (HeadlessPalette *)p;
    short i;
    if (!pal || !colors || n <= 0) return;
    if (pal->count + n > pal->size) {
        int newSize = (pal->count + n) * 2;
        COLOR *grown = (COLOR *)realloc(pal->colors, (size_t)newSize * sizeof(COLOR));
        if (!grown) return;
        pal->colors = grown;
        pal->size = newSize;
    }
    for (i = 0; i < n; i++) pal->colors[pal->count++] = colors[i];
}

short xvt_palet_get_colors(XVT_PALETTE p, COLOR *colorsp, short maxcolors)
{
    HeadlessPalette *pal = (HeadlessPalette *)p;
    short i, n;
    if (!pal || !colorsp) return 0;
    n = (short)(pal->count < maxcolors ? pal->count : maxcolors);
    for (i = 0; i < n; i++) colorsp[i] = pal->colors[i];
    return n;
}

/* ==========================================================================
 * Clipboard / text edit / timers / help / printing -- unavailable
 * ========================================================================== */
BOOLEAN xvt_cb_open(BOOLEAN for_write) { (void)for_write; return FALSE; }
BOOLEAN xvt_cb_close(void) { return TRUE; }
DATA_PTR xvt_cb_alloc_data(size_t size) { (void)size; return NULL; }
void xvt_cb_free_data(void) {}
BOOLEAN xvt_cb_put_data(long format, const char *name, long size, PICTURE data) { (void)format; (void)name; (void)size; (void)data; return FALSE; }
PICTURE xvt_cb_get_data(long format, const char *name, long *size) { (void)format; (void)name; if (size) *size = 0; return 0; }

long xvt_tx_process_event(WINDOW win, EVENT *ev) { (void)win; (void)ev; return 0L; }
void xvt_tx_suspend(WINDOW win) { (void)win; }
void xvt_tx_resume(WINDOW win) { (void)win; }
void xvt_tx_clear(WINDOW win) { (void)win; }
void xvt_tx_add_par(TXEDIT twin, T_PNUM line, const char *text) { (void)twin; (void)line; (void)text; }

DATA_PTR xvt_timer_create(WINDOW win, unsigned long ms) { (void)win; (void)ms; return NULL; }
void xvt_timer_destroy(DATA_PTR timer) { (void)timer; }

XVT_HELP_INFO xvt_help_open_helpfile(FILE_SPEC *fs, long flags) { (void)fs; (void)flags; return NULL; }
void xvt_help_close_helpfile(XVT_HELP_INFO hi) { (void)hi; }
BOOLEAN xvt_help_process_event(XVT_HELP_INFO hi, WINDOW win, EVENT *ev) { (void)hi; (void)win; (void)ev; return FALSE; }
void xvt_help_set_win_assoc(XVT_HELP_INFO hi, WINDOW win, long topic, long flags) { (void)hi; (void)win; (void)topic; (void)flags; }
void xvt_help_set_menu_assoc(XVT_HELP_INFO hi, WINDOW win, long tag, long topic, long flags) { (void)hi; (void)win; (void)tag; (void)topic; (void)flags; }

PRINT_RCD *xvt_print_create(long *size)
{
    if (size) *size = (long)sizeof(PRINT_RCD);
    return (PRINT_RCD *)calloc(1, sizeof(PRINT_RCD));
}
WINDOW xvt_print_create_win(PRINT_RCD *printer_setup, const char *title) { (void)printer_setup; (void)title; return NULL_WIN; }
void xvt_print_start_thread(DATA_PTR draw_fn, long user_data) { (void)draw_fn; (void)user_data; }
BOOLEAN xvt_print_open_page(PRINT_RCD *printer_setup) { (void)printer_setup; return FALSE; }
void xvt_print_close_page(PRINT_RCD *printer_setup) { (void)printer_setup; }
RCT *xvt_print_get_next_band(void) { return NULL; }

/* ==========================================================================
 * I/O streams: the FILE* itself is the stream (caller fcloses it)
 * ========================================================================== */
DATA_PTR xvt_iostr_create_fwrite(FILE *fp) { return (DATA_PTR)fp; }
void xvt_iostr_destroy(DATA_PTR iostr) { (void)iostr; }

/* ==========================================================================
 * Misc / screen
 * ========================================================================== */
void xvt_scr_set_focus_vobj(WINDOW win) { (void)win; }
WINDOW xvt_scr_get_focus_vobj(void) { return NULL_WIN; }

SLIST xvt_scr_list_wins(void)
{
    /* No top-level windows are ever shown: an empty list. */
    return xvt_slist_create();
}

void xvt_str_copy(char *dst, const char *src) { if (dst && src) strcpy(dst, src); }

void xvt_errmsg_sig_if(BOOLEAN cond, WINDOW win, int severity, long err, const char *arg1, long code, const char *msg)
{
    (void)win; (void)err; (void)code;
    if (!cond) return;
    fprintf(stderr, "XVT fatal assert (sev=%d, arg=%s): %s\n", severity, arg1 ? arg1 : "", msg ? msg : "");
    if (severity >= SEV_FATAL) abort();
}

/* ==========================================================================
 * Status bar (statbar.h)
 * ========================================================================== */
WINDOW statbar_create(int id, int left, int top, int right, int bottom,
                      int num_props, char **props, WINDOW parent,
                      int ctl_id, long style, const char *title)
{
    RCT rct;
    (void)id; (void)num_props; (void)props; (void)ctl_id; (void)style;
    xvt_rect_set(&rct, (short)left, (short)top, (short)right, (short)bottom);
    return allocHandle(W_PLAIN, &rct, title, parent, NULL, 0);
}
void statbar_set_default_title(WINDOW sb, const char *title) { (void)sb; (void)title; }
void statbar_set_title(WINDOW sb, const char *title) { (void)sb; (void)title; }
void statbar_autosize(WINDOW win) { (void)win; }
void statbar_set_cancel_visible(WINDOW sb, BOOLEAN visible) { (void)sb; (void)visible; }
