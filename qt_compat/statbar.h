/* ============================================================================
 * XVT -> Qt COMPATIBILITY LAYER  (flagged file: see qt_compat/README.md)
 *
 * Stand-in for XVT's status-bar control header (<statbar.h>). Real
 * implementations live in qt_compat/xvt_compat.cpp (backed by a genuine
 * QStatusBar) -- this header used to define these as `static inline`
 * no-op bodies directly, which meant every .c file that included it (via
 * nodInc.h, i.e. nearly the whole app) got its own dead-stub copy with
 * internal linkage, completely shadowing any real definition elsewhere no
 * matter what xvt_compat.cpp provided. That's why the main window never
 * showed status text at all (user-reported as "main window doesn't have
 * status info at bottom") -- plain declarations here now let the linker
 * resolve every call site to the one real definition, same as every other
 * xvt_* function in this compat layer.
 * ============================================================================
 */
#ifndef XVT_QT_COMPAT_STATBAR_H
#define XVT_QT_COMPAT_STATBAR_H

#include "xvt_types.h"

#ifdef __cplusplus
extern "C" {
#endif

WINDOW statbar_create(int id, int left, int top, int right, int bottom,
                       int num_props, char **props, WINDOW parent,
                       int ctl_id, long style, const char *title);
void statbar_set_default_title(WINDOW sb, const char *title);
void statbar_set_title(WINDOW sb, const char *title);
void statbar_autosize(WINDOW win);
/* todo.txt #83 -- shows/hides a "Cancel" button docked in the status bar's
 * permanent-widget area, for use while a long calculation (nodLib2.c's
 * initLongJob/incrementLongJob/finishLongJob) is running in place of the
 * old JOB_STATUS_WINDOW popup's own Cancel button. Clicking it calls
 * setAbortLongJob() directly (see xvt_compat.cpp) -- the one place this
 * compat layer calls back into application logic, since this button has
 * no ctlId/WIN_EVENT_HANDLER of its own to dispatch an E_CONTROL through. */
void statbar_set_cancel_visible(WINDOW sb, BOOLEAN visible);

#ifdef __cplusplus
}
#endif

#endif /* XVT_QT_COMPAT_STATBAR_H */
