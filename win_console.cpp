/* ============================================================================
 * [Qt port ADDITION] todo.txt #67 -- "windows version doesn't have noddy
 * icon for app" ...no wait, that's #64. This one: "Can we make the startup
 * console that launches noddy under windows silent (invisible) so it
 * behaves more like a normal windows app".
 *
 * MinGW links noddy.exe as a console-subsystem app (needed so
 * fprintf(stdout/stderr, ...) works for batch/CLI mode -- -block, -anom,
 * -random, etc, all write there), which means a plain console window
 * flashes up and stays open behind the GUI for ordinary interactive/
 * double-click launches, unlike a normal Windows app.
 *
 * This is a STANDALONE translation unit, deliberately isolated from
 * everything else in this codebase: <windows.h> defines BOOLEAN (as
 * `typedef BYTE BOOLEAN`) which directly conflicts with xvt_types.h's own
 * `typedef int BOOLEAN` the moment both are visible in the same
 * translation unit -- confirmed failing to compile both when included
 * directly from noddy.c (which pulls in nodStruc.h/3d.h, themselves
 * defining LineTo/INPUT with different meanings than <windows.h>'s own)
 * and when included from qt_compat/xvt_compat.cpp (which pulls in
 * qt_compat/xvt.h earlier in the same file -- order doesn't help, both
 * typedefs are already active). A separate .cpp with no other includes
 * from this codebase sidesteps the whole collision.
 * ============================================================================
 */
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

extern "C" void xvt_hide_console_if_owned(void)
{
    /* Only hide the console if this process is its SOLE owner (i.e.
     * Windows auto-allocated a fresh one because we were double-clicked/
     * launched with none of our own) -- never touch a console shared
     * with a parent shell (the rare `noddy -fn ...` interactive-with-a-
     * flag case still falls through to the GUI path in noddy.c's main(),
     * and hiding a console the user's own terminal is using would yank
     * their terminal window away from them). */
    DWORD consolePids[2];
    DWORD numConsoleProcs = GetConsoleProcessList(consolePids, 2);
    if (numConsoleProcs == 1)
    {
        HWND consoleWin = GetConsoleWindow();
        if (consoleWin)
            ShowWindow(consoleWin, SW_HIDE);
    }
}
#else
extern "C" void xvt_hide_console_if_owned(void) {}
#endif
