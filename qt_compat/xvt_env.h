/* ============================================================================
 * XVT -> Qt COMPATIBILITY LAYER  (flagged file: see qt_compat/README.md)
 *
 * Real XVT ships this file (documented in the XVT Portability Toolkit Guide,
 * sec 2.6, as "the file xvt_env.h in your include directory") defining the
 * window-system/file-system/OS/compiler symbols apps use for conditional
 * compilation, e.g. `#if XVTWS == MACWS`.
 *
 * We define XVTWS to a value distinct from every platform constant below,
 * so every one of this app's `#if XVTWS == MACWS` / `WIN32WS` / `XOLWS` /
 * `MTFWS` / ... branches evaluates false and compiles out -- all of that
 * code is platform-specific non-portable escape hatches (direct Win32/Mac
 * Toolbox/Motif calls) that Qt's own portability makes unnecessary. This is
 * a real behavioral consequence worth flagging: those branches are
 * deliberately never taken under this port, not calculation logic.
 * ============================================================================
 */
#ifndef XVT_QT_COMPAT_ENV_H
#define XVT_QT_COMPAT_ENV_H

#define MACWS   1
#define WINWS   2
#define WIN16WS 3
#define WIN32WS 4
#define XOLWS   5
#define MTFWS   6
#define PMWS    7
#define MAXWS   8   /* typo'd for MACWS at a couple of call sites in this codebase */

#define XVTWS   0   /* "the Qt window system" -- matches none of the above */

#define XVT_FILESYS_UNIX 1
#define XVT_FILESYS_DOS  0
#define XVT_FILESYS_MAC  0

#endif /* XVT_QT_COMPAT_ENV_H */
