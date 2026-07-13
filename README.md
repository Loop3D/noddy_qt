# Noddy (Qt port)

Noddy is a geological forward-modelling application originally developed at Imperial College, University of London (created 1981-2) and then updated at
Monash University (first released 1994), latest changes carried out at The University of Western Australia (see https://tectonique.net/noddy/ for more info). It builds a 3D geological model
from a user-specified sequence of tectonic/depositional events (folds,
faults, unconformities, dykes, plugs, ...), voxelises the result into a
block model, and forward-calculates gravity and magnetic anomalies from it.

The original GUI was built on **XVT**, a commercial/legacy cross-platform C
toolkit that requires a commerical licence. This tree has been ported to **Qt5**
via a compatibility shim (`qt_compat/`) that implements the subset of the
XVT API this application actually uses, on top of real Qt widgets. None of
the calculation code (event kinematics, voxelisation, geophysics forward
modelling) was not changed by the port -- only the GUI layer underneath it.

This document covers how to **compile** Noddy from scratch on Ubuntu,
Windows (MinGW), and macOS. For what needs to be installed/bundled to
**run** an already-built binary on another machine, see [user.txt](user.txt).


## Quick start

```
make -f Makefile.qt
```

run from the appropriate shell for your platform (see below) once its
prerequisites are installed. Produces `./noddy` (or `noddy.exe` on
Windows) in the repository root.

```
make -f Makefile.qt clean
```

removes all build output.

`Makefile.qt` auto-detects the host platform and adjusts Qt5 discovery,
the output filename, and a couple of compile/link flags accordingly -- the
same `make -f Makefile.qt` command is used on every platform.


## Ubuntu / Debian

Install a C/C++ toolchain and the Qt5 development packages:

```
sudo apt-get install build-essential qtbase5-dev qt5-qmake pkg-config
```

Then, from the repository root:

```
make -f Makefile.qt
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
   make -f Makefile.qt
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

**Known limitation**: a handful of `long`-as-pointer-carrier spots in the
Qt compat layer (`qt_compat/xvt_types.h`'s `PTR_LONG` macro backing
`xvt_vobj_set_data`/`get_data` and dialog "app data" passing, and the
`PICTURE` clipboard-descriptor typedef) only round-trip losslessly on
platforms where `long` is pointer-width -- true on Linux/Mac (LP64), NOT
on native 64-bit Windows (LLP64, where `long` stays 32 bits). In practice
this tends to work for typical small/early heap allocations, but isn't a
guarantee; see `PTR_LONG`'s own comment in `qt_compat/xvt_types.h` for
the full explanation. Not fixed as part of getting a Windows build
working -- would need widening `long` to `intptr_t` across every XVT API
signature that uses it for data-passing, a separate, larger audit.

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
   `Makefile.qt` adds Homebrew's Qt5 pkgconfig directory to
   `PKG_CONFIG_PATH` automatically (checking both the Apple Silicon
   `/opt/homebrew` and Intel `/usr/local` prefixes), so no manual
   `brew link --force qt@5` is needed.
3. From the repository root:
   ```
   make -f Makefile.qt
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
- There is no header-dependency tracking in `Makefile.qt`. If you edit a
  shared header (e.g. `nodStruc.h`, `qt_compat/xvt_types.h`) and a stale
  object file doesn't get rebuilt, `touch` the affected `.c`/`.cpp` files
  (or `make -f Makefile.qt clean` first) before rebuilding.

Note: the repository also contains a `MAKEFILE` (no extension) -- this is
the **original 1994 XVT-Design-generated makefile**, targeting a
SunOS/XVT/XOL toolchain that no longer exists, and its object-file list
references source files that have since been reorganised or removed. It
is kept for historical reference only; use `Makefile.qt` for an actual
build.


## Project layout

See [CLAUDE.md](CLAUDE.md) for a full description of the codebase's
architecture (the event-history model, voxelisation, geophysics forward
modelling, the 3D renderer, and how the Qt compatibility layer maps onto
the original XVT-based GUI).
