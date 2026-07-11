/* ============================================================================
 * XVT -> Qt COMPATIBILITY LAYER  (flagged file: see qt_compat/README.md)
 *
 * Stand-in for XVT's "Custom Control Manager" header. Most of this API is
 * genuinely inert here (xvtcm_eh_start/_end and friends exist in real XVT
 * to wrap a custom control's OWN event handler with generic bookkeeping
 * that this port's XvtWindow already does unconditionally for every
 * window) -- those stay real no-ops, not a missed port.
 *
 * xvtcm_create() is different: it's the ONLY thing that actually
 * constructs the custom control (graph.c's graph_create(), the Profile
 * window's point-editing curve canvas). Previously a no-op stub always
 * returning NULL_WIN, so the canvas was never created at all -- its
 * handler (graph_eh, with all the curve-drawing and point-drag logic)
 * never got attached to a real window (matches a user report of the
 * Profile curve editor's drawing panel being blank). Now backed by
 * xvt_custom_ctl_create() (qt_compat/xvt_compat.cpp), which makes a real
 * XvtWindow -- backing image, paintEvent, full mouse dispatch -- parented
 * and ctlId-tagged the same way every other control is.
 * ============================================================================
 */
#ifndef XVT_QT_COMPAT_XVTCM_H
#define XVT_QT_COMPAT_XVTCM_H

#include "xvt_types.h"
#include "xvt.h"

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
    RCT rct;
    (void)prop_count; (void)prop_list; (void)parent_rid; (void)parent_flags;
    (void)parent_class; (void)set_creation_args;
    rct.left = (short)left; rct.top = (short)top;
    rct.right = (short)right; rct.bottom = (short)bottom;
    return xvt_custom_ctl_create(cid, &rct, parent_win, eh);
}

/* Custom controls store an XVTCM_CONTROL_INFO as the first member of their
 * own per-control-instance struct; XVTCM_GET_INFO recovers a pointer to it
 * from the EVENT delivered to the control's event handler. */
#define XVTCM_GET_INFO(ev) (&(ev)->v.ctl.ci)

#ifdef __cplusplus
}
#endif

#endif /* XVT_QT_COMPAT_XVTCM_H */
