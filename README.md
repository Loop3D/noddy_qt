# Noddy (Qt port)

Noddy is a geological forward-modelling application originally developed at Imperial College, University of London (created 1981-2) and then updated at
Monash University (first released 1994), latest changes carried out at The University of Western Australia (see https://tectonique.net/noddy/ for more info). It builds a 3D geological model
from a user-specified sequence of tectonic/depositional events (folds,
faults, unconformities, dykes, plugs, ...), voxelises the result into a
block model, and forward-calculates gravity and magnetic anomalies from it.
## New Features
This port has allowed several new features to be added to the code:
- Clicked orientations on maps and sections now include x,y,z coordinates when saved to file
- Uniform bedding orientations and lithology can be saved as cvs for whole top surface of map (Save Surface Orientations... menu item)
- Random 5-event history generator added (Random History menu item)
- Voxels above topo surface in block diagrams can now be rendered transparent (In Display Type Dialog).
- Gaussian noise can now be added to density and magnetic susceptibility at voxel level (In Geophysics Calculation Options)

## Porting information

The original GUI was built on **XVT**, a  cross-platform C
toolkit that requires a commerical licence. This tree has been ported to **Qt5**
via a compatibility shim (`qt_compat/`) that implements the subset of the
XVT API this application actually uses, on top of real Qt widgets. None of
the calculation code (event kinematics, voxelisation, geophysics forward
modelling) were not changed by the port -- only the GUI layer underneath it, however some new code has since been added.

This document covers how to **compile** Noddy from scratch on Ubuntu,
Windows (MinGW), and macOS. For what needs to be installed/bundled to
**run** an already-built binary on another machine, see [user.txt](user.txt).


## Quick start

```
make
```

run from the appropriate shell for your platform (see below) once its
prerequisites are installed. Produces `./noddy` (or `noddy.exe` on
Windows) in the repository root.

```
make clean
```

removes all build output.

`Makefile` auto-detects the host platform and adjusts Qt5 discovery,
the output filename, and a couple of compile/link flags accordingly -- the
same `make` command is used on every platform.


## Ubuntu / Debian

Install a C/C++ toolchain and the Qt5 development packages:

```
sudo apt-get install build-essential qtbase5-dev qt5-qmake pkg-config
```

Then, from the repository root:

```
make
./noddy
```

To run in a headless environment with no display (e.g. CI, or this port's
own batch/CLI modes -- `-block`, `-anom`, etc):

```
QT_QPA_PLATFORM=offscreen ./noddy somefile.his -block
```


## Windows (MSYS2 / MinGW)

**Verified working** (native x86_64 build, MSYS2 MinGW64, gcc 15.1.0, Qt
5.15.17) -- both batch mode (`-block`, `-random`) and the interactive GUI
launch and run correctly.

The Windows build uses **MSYS2's MinGW64 environment**, not a native
Visual Studio/MSVC toolchain, and not a plain cmd.exe or PowerShell prompt.

1. Install [MSYS2](https://www.msys2.org/) if not already present.
2. Open the **"MSYS2 MinGW64"** shell specifically (not the plain "MSYS2"
   shell -- that one doesn't have the mingw-w64 toolchain on its PATH).
3. Install the toolchain and Qt5 (needs an elevated/Administrator MSYS2
   MinGW64 shell -- `pacman` writes to its package database under
   `<msys2 install dir>\var\lib\pacman`, which a non-admin account
   typically can't):
   ```
   pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-qt5-base \
             mingw-w64-x86_64-pkgconf make
   ```
4. From the repository root (inside that same MinGW64 shell -- doesn't
   need to be elevated for this step):
   ```
   make
   ./noddy.exe
   ```

The Makefile detects Windows via the `OS=Windows_NT` environment variable
(set regardless of shell) and adjusts accordingly: output is named
`noddy.exe`, `-fPIC` is dropped (meaningless for PE targets), and
`-mthreads` is added for correct exception unwinding across the Qt DLL
boundary. `CFLAGS` also carries `-Wno-implicit-function-declaration
-Wno-implicit-int` on every platform (not just Windows) -- this ~30-year
old codebase has always relied on old-C leniency for a handful of missing
standard-library includes and no-explicit-return-type function
definitions scattered through it; Linux's typically-older default GCC
only ever warned about these, but GCC 14+ (this MinGW toolchain) promotes
both to hard errors by default, which would otherwise block a Windows
build entirely over pre-existing, functionally-harmless omissions
unrelated to the Qt port itself.

**Fixed, but worth knowing about**: the app used to crash (access
violation) almost immediately on every native Windows launch, before any
window appeared. Root cause: several "attach a heap pointer as a window/
dialog's opaque user data" spots in the Qt compat layer (`xvt_vobj_set_data`
/`get_data`, `xvt_win_create`/`_def`, `xvt_dlg_create_res`/`_def`,
`createCenteredWindow`/`createCenteredDialog`/`createPositionedWindow`)
carried that pointer through a `long`, via the `PTR_LONG` macro in
`qt_compat/xvt_types.h`. `long` is pointer-width on Linux/Mac (LP64), so
this was invisible there -- but on native 64-bit Windows (LLP64, where
`long` stays 32 bits even though pointers are 64), it silently truncated
any heap pointer that didn't fit in 32 bits, corrupting it. `builder.c`'s
`setupWindow()` -- called for every window's very first setup, including
the main History window at startup -- hit this on essentially every
launch (confirmed via Windows Error Reporting crash dumps + `addr2line`
against the built `noddy.exe`, 100% reproducible at the same fault
offset). Fixed by widening the whole chain (and every app-side call site
passing a real pointer through it) from `long` to `intptr_t` -- see
`PTR_LONG`'s comment in `qt_compat/xvt_types.h` for the full chain. The
`PICTURE` clipboard-descriptor typedef (`xvt_cb_get_data`) has the
identical shape but was NOT widened -- clipboard paste isn't confirmed to
crash, and is a narrower, lower-traffic code path, so it's left as a
documented, smaller residual risk rather than growing this fix further.

If `./noddy.exe` fails to start with no error output (or Windows reports
it can't find an entry point / a DLL), it's almost always a missing Qt5
DLL on `PATH` -- see [user.txt](user.txt) for the exact DLL list and how
to bundle them so the binary runs outside the MSYS2 shell.


## macOS

1. Install Xcode's Command Line Tools (provides `g++`/`clang` and `make`):
   ```
   xcode-select --install
   ```
2. Install Qt5 via [Homebrew](https://brew.sh/):
   ```
   brew install qt@5 pkg-config
   ```
   Homebrew's `qt@5` formula is "keg-only" -- it's deliberately not linked
   onto the default PATH/pkg-config search path, since Homebrew doesn't
   want to presume this Qt version should be the system default.
   `Makefile` adds Homebrew's Qt5 pkgconfig directory to
   `PKG_CONFIG_PATH` automatically (checking both the Apple Silicon
   `/opt/homebrew` and Intel `/usr/local` prefixes), so no manual
   `brew link --force qt@5` is needed.
3. From the repository root:
   ```
   make
   ./noddy
   ```

To redistribute the binary to a machine without Homebrew/Qt5 installed, it
needs to be wrapped into a proper `.app` bundle and processed with Qt's
`macdeployqt` tool -- see the macOS section of [user.txt](user.txt).


## How the build works

- Every original application `.c` file is compiled as **plain C** (via
  `gcc`) -- most of this 1990s/2000s codebase uses old K&R-style function
  definitions that a C++ compiler rejects, so these files are compiled
  as C, unmodified, exactly as they always were.
- Only the Qt compatibility layer (`qt_compat/xvt_compat.cpp` and friends)
  is C++ (`-std=c++17`, via `g++`), since it talks to real Qt5 widgets.
- The final link step uses `g++` so the C++/Qt runtime is linked in
  correctly, even though almost every object file was compiled as C.
- `noddylic.c` is excluded from the build -- it's a separate standalone
  license-file-generator utility with its own `main()`, not part of the
  `noddy` binary.
- There is no header-dependency tracking in `Makefile`. If you edit a
  shared header (e.g. `nodStruc.h`, `qt_compat/xvt_types.h`) and a stale
  object file doesn't get rebuilt, `touch` the affected `.c`/`.cpp` files
  (or `make clean` first) before rebuilding.

Note: this `Makefile` was originally `Makefile.qt`, renamed once it became
the only build path worth using so plain `make` (no `-f`) works out of the
box. The repository used to also contain a `MAKEFILE` (no extension) --
the **original 1994 XVT-Design-generated makefile**, targeting a
SunOS/XVT/XOL toolchain that no longer exists, with an object-file list
referencing source files long since reorganised or removed. It has been
removed; this `Makefile` is the only build path now.


## Project layout

See [CLAUDE.md](CLAUDE.md) for a full description of the codebase's
architecture (the event-history model, voxelisation, geophysics forward
modelling, the 3D renderer, and how the Qt compatibility layer maps onto
the original XVT-based GUI).

See [MODIFICATIONS.md](MODIFICATIONS.md) for a function-by-function list of
every change made to the original application logic during the port (bug
fixes, small new features, default-value changes) as opposed to the
mechanical toolkit-call swap the port otherwise consists of.


## Qt distribution
Qt is used to build the GUI for this code. Distribution of Qt libraries for Open Source codes is allowed as long as the source code for the Qt libraries is made available (here https://download.qt.io/). The Qt code is ditributed under a LGPLv3 license [lgpl-3.0](lgpl-3.0.txt).