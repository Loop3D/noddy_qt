/* ============================================================================
 * XVT -> Qt COMPATIBILITY LAYER  (flagged file: see qt_compat/README.md)
 *
 * This is the drop-in replacement for the original proprietary "xvt.h" SDK
 * header that every .c file in this application includes. It declares the
 * same ~190 xvt_xxx() functions the application already calls; their
 * *implementations* (in xvt_compat.cpp) are backed by Qt5 Widgets instead of
 * the original XVT/Motif toolkit. Nothing in the ~200 application .c/.h
 * files changed to make this port work -- only this header and
 * xvt_compat.cpp are new.
 *
 * Coverage status of the declarations below (see xvt_compat.cpp for which
 * are genuinely implemented vs. temporary stubs):
 *   [DONE]  = backed by real Qt logic
 *   [STUB]  = declared + linkable, logs a "not yet ported" warning at
 *             runtime and returns a harmless default; needed so the whole
 *             ~200-file application links today while porting continues
 *             incrementally.
 * Grep xvt_compat.cpp for XVT_STUB to see the current stub list.
 * ============================================================================
 */
#ifndef XVT_QT_COMPAT_H
#define XVT_QT_COMPAT_H

#include "xvt_types.h"
#include <stdio.h> /* FILE -- xvt_iostr_create_fwrite takes an already-open FILE* */

#ifdef __cplusplus
extern "C" {
#endif

/* ==================== Application / task window ========================= */
extern XVT_CONFIG xvt_config;                          /* [DONE] app defines this global, matches real XVT */
int  xvt_app_create(int argc, char **argv, long flags, WIN_EVENT_HANDLER task_eh, XVT_CONFIG *cfg); /* [DONE] */
void xvt_app_destroy(void);                             /* [DONE] */
BOOLEAN xvt_app_allow_quit(void);                        /* [DONE] */
void xvt_app_process_pending_events(void);               /* [DONE] */
void xvt_app_get_default_ctools(DRAW_CTOOLS *tools);     /* [STUB] */

/* ==================== vobj: generic object attributes/data =============== */
long xvt_vobj_get_attr(WINDOW win, XVT_ATTR attr);       /* [DONE] */
void xvt_vobj_set_attr(WINDOW win, XVT_ATTR attr, long value); /* [DONE] */
long xvt_vobj_get_data(WINDOW win);                      /* [DONE] */
void xvt_vobj_set_data(WINDOW win, long data);           /* [DONE] */
void xvt_vobj_destroy(WINDOW win);                       /* [DONE] */
void xvt_vobj_set_enabled(WINDOW win, BOOLEAN enabled);  /* [DONE] */
BOOLEAN xvt_vobj_get_enabled(WINDOW win);                /* [DONE] */
void xvt_vobj_set_visible(WINDOW win, BOOLEAN visible);  /* [DONE] */
BOOLEAN xvt_vobj_get_visible(WINDOW win);                /* [DONE] */
void xvt_vobj_set_title(WINDOW win, const char *title);  /* [DONE] */
BOOLEAN xvt_vobj_get_title(WINDOW win, char *buf, int maxlen); /* [DONE] */
void xvt_vobj_move(WINDOW win, RCT *rct);                /* [DONE] */
void xvt_vobj_get_client_rect(WINDOW win, RCT *rct);     /* [DONE] */
void xvt_vobj_get_outer_rect(WINDOW win, RCT *rct);      /* [DONE] */
WINDOW xvt_vobj_get_parent(WINDOW win);                  /* [DONE] */
WIN_TYPE xvt_vobj_get_type(WINDOW win);                  /* [DONE] */
void xvt_vobj_translate_points(WINDOW from, WINDOW to, PNT *pts, int n); /* [DONE] */
void xvt_vobj_raise(WINDOW win);                         /* [STUB] */
void xvt_vobj_set_palet(WINDOW win, DATA_PTR palet);     /* [STUB] */
DATA_PTR xvt_vobj_get_palet(WINDOW win);                 /* [STUB] */

/* ==================== Windows / dialogs =================================== */
WINDOW xvt_win_create(WIN_TYPE type, RCT *rct, const char *title, long menu_res_id,
                      WINDOW parent, unsigned long style, EVENT_MASK mask,
                      WIN_EVENT_HANDLER eh, long user_data);         /* [DONE] */
WINDOW xvt_win_create_def(WIN_DEF *def, WINDOW parent, EVENT_MASK mask,
                      WIN_EVENT_HANDLER eh, long user_data);         /* [DONE] */
WINDOW xvt_win_get_ctl(WINDOW win, int ctl_id);          /* [DONE] */
void xvt_win_set_cursor(WINDOW win, int cursor_id);      /* [STUB] */
long xvt_win_dispatch_event(WINDOW win, EVENT *ev);      /* [DONE] */
TXEDIT xvt_win_get_tx(WINDOW win, int ctl_id);           /* [STUB] */
void xvt_win_trap_pointer(WINDOW win);                   /* [STUB] */
void xvt_win_release_pointer(void);                      /* [STUB] */

WINDOW xvt_dlg_create_res(WIN_TYPE dlg_type, long dlg_res_id, EVENT_MASK mask,
                      WIN_EVENT_HANDLER eh, long user_data);         /* [DONE] */
WINDOW xvt_dlg_create_def(WIN_DEF *def, EVENT_MASK mask,
                      WIN_EVENT_HANDLER eh, long user_data);         /* [STUB] */

/* ==================== Controls ============================================= */
WINDOW xvt_ctl_create(WIN_TYPE type, RCT *rct, const char *title, WINDOW parent,
                      long flags, long font_id, int ctl_id);         /* [STUB] */
WINDOW xvt_custom_ctl_create(int cid, RCT *rct, WINDOW parent, WIN_EVENT_HANDLER eh); /* [DONE] -- backs xvtcm_create(), see qt_compat/xvtcm.h */
void xvt_win_set_ctl_proxy(WINDOW win, WINDOW proxyWin); /* [DONE] -- compat-layer-only bridge, see XvtObj::ctlProxyWin in xvt_compat.cpp */
void xvt_win_mark_controls_only(WINDOW win); /* [DONE] -- compat-layer-only, see XvtObj::controlsOnly in xvt_compat.cpp */
void xvt_win_set_redraw_proxy(WINDOW win, WINDOW proxyWin); /* [DONE] -- compat-layer-only, see XvtObj::redrawProxyWin in xvt_compat.cpp */
int xvt_win_get_menubar_height(WINDOW win); /* [DONE] -- compat-layer-only, see xvt_vobj_move's comment in xvt_compat.cpp */
BOOLEAN xvt_ctl_is_checked(WINDOW ctl_win);              /* [DONE] */
void xvt_ctl_set_checked(WINDOW ctl_win, BOOLEAN checked); /* [DONE] */
void xvt_ctl_check_radio_button(WINDOW checked, WINDOW *group, int count); /* [DONE] */
long xvt_ctl_get_id(WINDOW ctl_win);                     /* [STUB] */

/* ==================== Lists (list boxes) =================================== */
void xvt_list_add(WINDOW list_win, int index, const char *text);    /* [DONE] */
void xvt_list_clear(WINDOW list_win);                    /* [DONE] */
int  xvt_list_get_sel_index(WINDOW list_win);            /* [DONE] */
BOOLEAN xvt_list_get_first_sel(WINDOW list_win, char *buf, int maxlen); /* [DONE] gets the TEXT of the first selected item, not its index (despite the name) */
BOOLEAN xvt_list_set_sel(WINDOW list_win, int index, BOOLEAN sel);   /* [DONE] */
BOOLEAN xvt_list_is_sel(WINDOW list_win, int index);     /* [DONE] */
int  xvt_list_count_all(WINDOW list_win);                /* [DONE] */
void xvt_list_suspend(WINDOW list_win);                  /* [DONE] */
void xvt_list_resume(WINDOW list_win);                   /* [DONE] */

/* ==================== Scrollbars ============================================ */
/* Signatures fixed to match the real XVT SDK header (xvtsbar.h): all int,
 * not long -- xvt_sbar_get_range previously wrote through long* (8 bytes)
 * into call sites' `int minValue, maxValue;` locals (4 bytes each on
 * LP64 Linux), corrupting the caller's stack canary (SIGABRT "stack
 * smashing detected" in setScrollPosition, nodLib1.c, hit whenever any
 * Options dialog opened -- optnlib.c's loadGeologyDisplayOptions calls
 * updateScrollAndText -> setScrollPosition -> xvt_sbar_get_range). */
int  xvt_sbar_get_pos(WINDOW win, SCROLL_TYPE axis);             /* [DONE] */
void xvt_sbar_set_pos(WINDOW win, SCROLL_TYPE axis, int pos);   /* [DONE] */
void xvt_sbar_set_range(WINDOW win, SCROLL_TYPE axis, int lo, int hi); /* [DONE] */
void xvt_sbar_get_range(WINDOW win, SCROLL_TYPE axis, int *lo, int *hi); /* [DONE] */

/* ==================== Menus ================================================= */
void xvt_menu_set_item_enabled(WINDOW win, MENU_TAG tag, BOOLEAN enabled); /* [DONE] */
void xvt_menu_set_item_checked(WINDOW win, MENU_TAG tag, BOOLEAN checked); /* [STUB] */
void xvt_menu_set_tree(WINDOW win, DATA_PTR tree);       /* [STUB] */
DATA_PTR xvt_menu_get_tree(WINDOW win);                  /* [STUB] */
void xvt_menu_update(WINDOW win);                        /* [STUB] */

/* ==================== Message dialogs (xvt_dm_*) =========================== */
void xvt_dm_post_error(const char *fmt, ...);            /* [DONE] */
void xvt_dm_post_note(const char *fmt, ...);              /* [DONE] */
int  xvt_dm_post_ask(const char *b1, const char *b2, const char *b3, const char *fmt, ...); /* [DONE] */
FL_STATUS xvt_dm_post_file_open(FILE_SPEC *fs, const char *prompt); /* [DONE] */
FL_STATUS xvt_dm_post_file_save(FILE_SPEC *fs, const char *prompt); /* [DONE] */
BOOLEAN xvt_dm_post_string_prompt(const char *prompt, char *buf, int maxlen); /* [STUB] */
BOOLEAN xvt_dm_post_page_setup(PRINT_RCD *printer_setup); /* [DONE] */

/* ==================== Drawing (xvt_dwin_*) ================================= */
void xvt_dwin_clear(WINDOW win, COLOR color);            /* [DONE] */
void xvt_dwin_update(WINDOW win);                        /* [DONE] */
void xvt_dwin_invalidate_rect(WINDOW win, RCT *rct);      /* [DONE] */
BOOLEAN xvt_dwin_is_update_needed(WINDOW win, RCT *rct); /* [DONE] */
void xvt_dwin_draw_set_pos(WINDOW win, PNT pos);          /* [DONE] */
void xvt_dwin_draw_line(WINDOW win, PNT pos);             /* [DONE] */
void xvt_dwin_draw_aline(WINDOW win, PNT to, BOOLEAN close, BOOLEAN visible); /* [STUB] draws from the current pen position to `to`; close/visible affect fill+dash on real XVT, ignored here */
void xvt_dwin_draw_rect(WINDOW win, RCT *rct);            /* [DONE] */
void xvt_dwin_draw_roundrect(WINDOW win, RCT *rct, int ovalw, int ovalh); /* [STUB] */
void xvt_dwin_draw_oval(WINDOW win, RCT *rct);            /* [STUB] */
void xvt_dwin_draw_polygon(WINDOW win, PNT *pts, int n);  /* [DONE] */
void xvt_dwin_draw_polyline(WINDOW win, PNT *pts, int n); /* [STUB] */
void xvt_dwin_draw_text(WINDOW win, short x, short y, const char *text, int len); /* [DONE] */
int  xvt_dwin_get_text_width(WINDOW win, const char *text, int len); /* [DONE] */
int  xvt_dwin_text_width(WINDOW win, const char *text, int len); /* [STUB] */
void xvt_dwin_draw_icon(WINDOW win, short x, short y, int icon_id); /* [STUB] */
void xvt_dwin_draw_pmap(WINDOW win, XVT_PIXMAP pmap, RCT *dst, RCT *src); /* [DONE] */
/* Parameter order fixed to match the real XVT SDK header (xvtdwin.h):
 * image comes BEFORE dst/src, not between them -- the prior (win, RCT*,
 * DATA_PTR, RCT*) order didn't match any real call site's actual
 * argument order (all pass (pixmap, image, &rectA, &rectB)), which would
 * have silently reinterpreted an XVT_IMAGE handle as an RCT* the moment
 * this was implemented for real instead of a no-op stub. */
void xvt_dwin_draw_image(WINDOW win, XVT_IMAGE image, RCT *dst, RCT *src); /* [DONE] */
void xvt_dwin_scroll_rect(WINDOW win, RCT *rct, int dh, int dv); /* [STUB] */
void xvt_dwin_set_clip(WINDOW win, RCT *rct);             /* [STUB] */
void xvt_dwin_set_fore_color(WINDOW win, COLOR color);    /* [DONE] */
void xvt_dwin_set_back_color(WINDOW win, COLOR color);    /* [STUB] */
void xvt_dwin_set_font_family(WINDOW win, const char *family); /* [STUB] */
void xvt_dwin_set_font_style(WINDOW win, int style);      /* [DONE] */
void xvt_dwin_set_font_size(WINDOW win, int size);        /* [STUB] */
void xvt_dwin_set_font(WINDOW win, DATA_PTR font);        /* [STUB] */
DATA_PTR xvt_dwin_get_font(WINDOW win);                   /* [STUB] */
void xvt_dwin_set_draw_mode(WINDOW win, int mode);        /* [STUB] */
void xvt_dwin_set_cpen(WINDOW win, CPEN *pen);            /* [DONE] */
void xvt_dwin_set_std_cpen(WINDOW win, int std_pen);      /* [STUB] */
void xvt_dwin_set_std_cbrush(WINDOW win, int std_brush);  /* [STUB] */
void xvt_dwin_get_font_metrics(WINDOW win, int *leading, int *ascent, int *descent); /* [DONE] */
/* Signature fixed to match the real XVT SDK header (xvtfont.h): 4 args
 * (font_id, leadingp, ascentp, descentp), not 2 -- the prior 2-arg form
 * couldn't even compile against its one real call site (3dText.c, 4
 * args) were that call site ever live; it's dead code today (guarded by
 * `#ifdef PAUL`, a macro never #defined anywhere in this tree) but fixed
 * for correctness/insurance regardless. Still a stub (no live call site
 * to drive real implementation from). */
void xvt_font_get_metrics(DATA_PTR font, int *leadingp, int *ascentp, int *descentp); /* [STUB] */

void xvt_dwin_set_draw_ctools(WINDOW win, DRAW_CTOOLS *tools);  /* [DONE] */
void xvt_dwin_get_draw_ctools(WINDOW win, DRAW_CTOOLS *tools);  /* [DONE] */

/* ==================== Pixmaps / images ====================================== */
XVT_PIXMAP xvt_pmap_create(WINDOW win, int type, short width, short height, DATA_PTR palette); /* [DONE] */
void xvt_pmap_destroy(XVT_PIXMAP pmap);                   /* [DONE] */
XVT_IMAGE xvt_image_create(int type, short width, short height, DATA_PTR clut); /* [STUB] */
void xvt_image_destroy(XVT_IMAGE image);                   /* [STUB] */
XVT_IMAGE xvt_image_read(const char *filename);            /* [STUB] */
void xvt_image_get_dimensions(XVT_IMAGE image, short *w, short *h); /* [STUB] */
void xvt_image_set_pixel(XVT_IMAGE image, short x, short y, COLOR c); /* [STUB] */
COLOR xvt_image_get_pixel(XVT_IMAGE image, short x, short y);  /* [STUB] */
void xvt_image_get_from_pmap(XVT_IMAGE image, XVT_PIXMAP pmap, RCT *dst, RCT *src); /* [STUB] */
void xvt_image_set_ncolors(XVT_IMAGE image, int n);        /* [STUB] */
void xvt_image_set_clut(XVT_IMAGE image, COLOR *clut, int n); /* [STUB] */
BOOLEAN xvt_image_write_bmp_to_iostr(XVT_IMAGE image, DATA_PTR iostr); /* [STUB] */
BOOLEAN xvt_image_write_macpict_to_iostr(XVT_IMAGE image, DATA_PTR iostr); /* [STUB] */

/* ==================== Rects / geometry ====================================== */
void xvt_rect_set(RCT *r, short left, short top, short right, short bottom); /* [DONE] */
void xvt_rect_set_pos(RCT *r, PNT pos);                   /* [DONE] */
void xvt_rect_offset(RCT *r, short dh, short dv);         /* [DONE] */
void xvt_rect_set_width(RCT *r, short w);                 /* [STUB] */
void xvt_rect_set_height(RCT *r, short h);                /* [STUB] */
short xvt_rect_get_width(const RCT *r);                   /* [DONE] */
short xvt_rect_get_height(const RCT *r);                  /* [DONE] */
BOOLEAN xvt_rect_has_point(const RCT *r, PNT pt);         /* [DONE] */
BOOLEAN xvt_rect_intersect(RCT *dst, const RCT *a, const RCT *b); /* [STUB] */

/* ==================== Memory ================================================ */
DATA_PTR xvt_mem_alloc(size_t size);                      /* [DONE] */
DATA_PTR xvt_mem_zalloc(size_t size);                     /* [DONE] */
DATA_PTR xvt_mem_realloc(DATA_PTR p, size_t size);        /* [DONE] */
void xvt_mem_free(DATA_PTR p);                            /* [DONE] */
#define xvt_mem_hfree(X) xvt_mem_free(X)
DATA_PTR xvt_mem_halloc(long count, size_t elemsize);     /* [DONE] alias of calloc (no separate handle-based heap in this port) */
/* xvt_mem_zhalloc: NOT defined here -- application file 3d.h already
 * `#define`s it (`xvt_mem_zalloc((X)*(Y))`) wherever it's used, so defining
 * it again here would just be a harmless-but-noisy macro redefinition. */
DATA_PTR xvt_mem_rep(DATA_PTR p, size_t oldsize, size_t newsize); /* [STUB] */

/* ==================== Singly-linked lists (XVT SLIST) =======================
 * SLIST is an opaque list handle; SLIST_ELT is an opaque per-node handle
 * within it. `xvt_slist_get_first`/`_get_next` walk the node handles;
 * `xvt_slist_get` copies the long payload of a given node into `*data`. */
typedef struct s_xvt_slist *SLIST;
typedef struct s_xvt_slist_elt *SLIST_ELT;
SLIST xvt_slist_create(void);                             /* [STUB] */
void xvt_slist_destroy(SLIST l);                          /* [DONE] */
SLIST_ELT xvt_slist_get_first(SLIST l);                   /* [DONE] */
SLIST_ELT xvt_slist_get_next(SLIST l, SLIST_ELT elt);     /* [DONE] */
char *xvt_slist_get(SLIST l, SLIST_ELT elt, long *data);  /* [DONE] returns the node's associated name string, also filling *data with its long payload */
void xvt_slist_add_sorted(SLIST l, const char *name, long data, BOOLEAN unique, BOOLEAN case_sensitive); /* [STUB] */
long xvt_slist_count(SLIST l);                             /* [STUB] */

/* ==================== File system ============================================ */
BOOLEAN xvt_fsys_get_default_dir(DIRECTORY *dir);         /* [DONE] */
BOOLEAN xvt_fsys_set_dir(DIRECTORY *dir);                 /* [DONE] */
BOOLEAN xvt_fsys_get_dir(DIRECTORY *dir);                 /* [DONE] */
BOOLEAN xvt_fsys_set_dir_startup(void);                   /* [STUB] */
BOOLEAN xvt_fsys_convert_str_to_dir(const char *str, DIRECTORY *dir); /* [DONE] */
BOOLEAN xvt_fsys_convert_dir_to_str(DIRECTORY *dir, char *buf, int maxlen); /* [DONE] */
BOOLEAN xvt_fsys_get_file_attr(FILE_SPEC *fs, unsigned long attr); /* [DONE] */
/* 3rd param fixed from BOOLEAN to long: every real call site passes a Mac
 * Finder type/creator 4-char-code string pointer cast to long (e.g.
 * `(long)"TEXT"`), which BOOLEAN would truncate on LP64 -- correctly a
 * no-op regardless (no Linux/Windows equivalent of Mac file type/creator
 * metadata), see xvt_compat.cpp. */
BOOLEAN xvt_fsys_set_file_attr(FILE_SPEC *fs, unsigned long attr, long value); /* [DONE] */
BOOLEAN xvt_fsys_rem_file(FILE_SPEC *fs);                 /* [DONE] */
#define XVT_FILE_ATTR_READ 1
#define XVT_FILE_ATTR_DIRECTORY 2
#define XVT_FILE_ATTR_TYPESTR 3
#define XVT_FILE_ATTR_CTIME 4
#define XVT_FILE_ATTR_CREATORSTR 5

/* ==================== Resources / icons ====================================== */
typedef struct { int placeholder; } ICON_RESOURCE;
typedef struct { char *tag; long id; char *data; } RESOURCE_INFO;
void xvt_xres_build_icon(ICON_RESOURCE *iconp, unsigned h, unsigned w, DATA_PTR data); /* [STUB] */
RESOURCE_INFO *xvt_xres_create_table(void);               /* application-supplied on real XVT; unused under Qt (icons loaded from .ICO files directly) */
void xvt_res_free_menu_tree(DATA_PTR tree);                /* [STUB] */
void xvt_res_free_win_def(WIN_DEF *def);                    /* [DONE] frees the placeholder array from xvt_res_get_win_def */
WIN_DEF *xvt_res_get_win_def(long res_id);                  /* [STUB-placeholder] see qt_compat/README.md: real per-dialog layouts aren't in this repo; returns a minimal 1-window/no-controls WIN_DEF so callers don't crash */
WIN_DEF *xvt_res_get_dlg_def(long res_id);                  /* [STUB-placeholder] same as xvt_res_get_win_def */
DATA_PTR xvt_res_get_menu(long res_id);                    /* [STUB] */

/* ==================== Palettes ================================================ */
typedef struct { int placeholder; } XVT_PALETTE_REC; typedef XVT_PALETTE_REC *XVT_PALETTE;
enum { XVT_PALETTE_USER, XVT_PALETTE_STOCK, XVT_PALETTE_CURRENT };
XVT_PALETTE xvt_palet_create(int type, DATA_PTR reserved);  /* [DONE] */
void xvt_palet_destroy(XVT_PALETTE p);                     /* [DONE] */
void xvt_palet_set_tolerance(XVT_PALETTE p, long tol);     /* [DONE] */
void xvt_palet_add_colors(XVT_PALETTE p, COLOR *colors, short n); /* [DONE] */
short xvt_palet_get_colors(XVT_PALETTE p, COLOR *colorsp, short maxcolors); /* [DONE] -- real signature (fills caller buffer), confirmed against actual XVT SDK xvtpalet.h */

/* ==================== Clipboard ================================================ */
BOOLEAN xvt_cb_open(BOOLEAN for_write);                     /* [STUB] */
BOOLEAN xvt_cb_close(void);                                 /* [STUB] */
DATA_PTR xvt_cb_alloc_data(size_t size);                   /* [STUB] */
void xvt_cb_free_data(void);                                /* [STUB] */
BOOLEAN xvt_cb_put_data(long format, const char *name, long size, PICTURE data); /* [STUB] */
PICTURE xvt_cb_get_data(long format, const char *name, long *size); /* [STUB] */

/* ==================== Text-edit control (xvt_tx_*) ============================= */
long xvt_tx_process_event(WINDOW win, EVENT *ev);           /* [DONE] (delegates to native Qt line/text edit) */
void xvt_tx_suspend(WINDOW win);                            /* [STUB] */
void xvt_tx_resume(WINDOW win);                             /* [STUB] */
void xvt_tx_clear(WINDOW win);                              /* [STUB] */
void xvt_tx_add_par(TXEDIT twin, T_PNUM line, const char *text); /* [STUB] */

/* ==================== Timers ==================================================== */
DATA_PTR xvt_timer_create(WINDOW win, unsigned long ms);     /* [STUB] */
void xvt_timer_destroy(DATA_PTR timer);                      /* [STUB] */

/* ==================== Help ======================================================= */
typedef DATA_PTR XVT_HELP_INFO;
XVT_HELP_INFO xvt_help_open_helpfile(FILE_SPEC *fs, long flags); /* [STUB] */
void xvt_help_close_helpfile(XVT_HELP_INFO hi);              /* [STUB] */
BOOLEAN xvt_help_process_event(XVT_HELP_INFO hi, WINDOW win, EVENT *ev); /* [STUB] */
void xvt_help_set_win_assoc(XVT_HELP_INFO hi, WINDOW win, long topic, long flags); /* [STUB] */
void xvt_help_set_menu_assoc(XVT_HELP_INFO hi, WINDOW win, long tag, long topic, long flags); /* [STUB] */

/* ==================== Printing ==================================================== */
PRINT_RCD *xvt_print_create(long *size);                     /* [DONE] */
WINDOW xvt_print_create_win(PRINT_RCD *printer_setup, const char *title); /* [DONE] */
void xvt_print_start_thread(DATA_PTR draw_fn, long user_data); /* [DONE] */
BOOLEAN xvt_print_open_page(PRINT_RCD *printer_setup);       /* [DONE] */
void xvt_print_close_page(PRINT_RCD *printer_setup);         /* [DONE] */
RCT *xvt_print_get_next_band(void);                          /* [DONE] */

/* ==================== I/O streams (xvt_iostr_*) ================================== */
DATA_PTR xvt_iostr_create_fwrite(FILE *fp);                  /* [DONE] */
void xvt_iostr_destroy(DATA_PTR iostr);                      /* [DONE] */

/* ==================== Misc / screen ================================================ */
void xvt_scr_set_focus_vobj(WINDOW win);                     /* [DONE] */
WINDOW xvt_scr_get_focus_vobj(void);                         /* [STUB] */
SLIST xvt_scr_list_wins(void);                               /* [DONE] returns a SLIST of top-level WINDOW handles (as long payloads), matching xvt_slist_get's `long *` output convention */
void xvt_str_copy(char *dst, const char *src);               /* [DONE] thin strcpy wrapper, matches XVT_STR_MAXLEN-checked original */
void xvt_errmsg_sig_if(BOOLEAN cond, WINDOW win, int severity, long err, const char *arg1, long code, const char *msg); /* [DONE] */
#define SEV_FATAL 3
#define ERR_ASSERT_4 4

#ifdef __cplusplus
}
#endif

#endif /* XVT_QT_COMPAT_H */
