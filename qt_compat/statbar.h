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

#ifdef __cplusplus
}
#endif

#endif /* XVT_QT_COMPAT_STATBAR_H */
