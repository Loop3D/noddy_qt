/* ============================================================================
 * XVT -> Qt COMPATIBILITY LAYER  (flagged file: see qt_compat/README.md)
 *
 * Stand-in for XVT's "Custom Control Manager" header. Only a handful of
 * call sites in this codebase (custom-control creation glue in
 * builder.c/rmbuild.c and the one-time app-startup call in noddy.c) use
 * this API; none of it is calculation code. [STUB] -- no-ops for now.
 * ============================================================================
 */
#ifndef XVT_QT_COMPAT_XVTCM_H
#define XVT_QT_COMPAT_XVTCM_H

#include "xvt_types.h"

#ifdef __cplusplus
extern "C" {
#endif

static inline void xvtcm_set_src_char_size(PNT size) { (void)size; }
static inline void xvtcm_eh_start(WINDOW win, EVENT *ev) { (void)win; (void)ev; }
static inline void xvtcm_eh_end(WINDOW win, EVENT *ev) { (void)win; (void)ev; }
static inline void xvtcm_parent_event(WINDOW win, EVENT *ev) { (void)win; (void)ev; }
static inline WINDOW xvtcm_create(int cid, int left, int top, int right, int bottom,
                                   int prop_count, char **prop_list, WINDOW parent_win,
                                   int parent_rid, long parent_flags, char *parent_class,
                                   WIN_EVENT_HANDLER eh, void (*set_creation_args)(WIN_CREATION_ARGS *))
{
    (void)cid; (void)left; (void)top; (void)right; (void)bottom; (void)prop_count;
    (void)prop_list; (void)parent_win; (void)parent_rid; (void)parent_flags;
    (void)parent_class; (void)eh; (void)set_creation_args;
    return NULL_WIN;
}

/* Custom controls store an XVTCM_CONTROL_INFO as the first member of their
 * own per-control-instance struct; XVTCM_GET_INFO recovers a pointer to it
 * from the EVENT delivered to the control's event handler. */
#define XVTCM_GET_INFO(ev) (&(ev)->v.ctl.ci)

#ifdef __cplusplus
}
#endif

#endif /* XVT_QT_COMPAT_XVTCM_H */
