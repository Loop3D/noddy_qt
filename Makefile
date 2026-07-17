# ============================================================================
# Modern g++ / Qt5 Makefile for the Noddy Qt port.
#
# Replaces the original 1994 XVT/XOL `MAKEFILE` (removed from this tree --
# see CLAUDE.md -- it targeted a SunOS/XVT toolchain that no longer exists).
# This file was itself originally named `Makefile.qt`; renamed to the
# standard `Makefile` (todo.txt #63) so plain `make` finds it without
# `-f Makefile.qt`, now that it's the only build path in the tree.
# This builds the SAME application source files unchanged, linked against
# qt_compat/ instead of the proprietary XVT SDK.
#
# [Qt port change] todo.txt #57: cross-platform support (was Linux/pkg-config
# only). Detects the host platform (Linux, macOS, or Windows via MSYS2/MinGW)
# and adjusts the Qt5 discovery method, executable name, and a couple of
# platform-specific link/compile flags accordingly. The actual build recipes
# (compile each .c as C, link everything with g++) are UNCHANGED across
# platforms -- only how Qt5's include/lib paths get discovered, and the
# output filename, differ.
#
# Requires: g++/gcc (C++17) and Qt5 development packages, per platform:
#
#   Linux (Debian/Ubuntu):
#     sudo apt-get install build-essential qtbase5-dev qt5-qmake pkg-config
#
#   macOS (Homebrew, either Intel or Apple Silicon):
#     brew install qt@5 pkg-config
#     (qt@5 is "keg-only" -- not symlinked onto the default PATH/pkg-config
#     search path -- this Makefile adds Homebrew's Qt5 pkgconfig directory
#     to PKG_CONFIG_PATH automatically; no manual `brew link` needed.)
#     Xcode Command Line Tools must be installed for a working g++/clang:
#     xcode-select --install
#
#   Windows (MSYS2 MinGW64 shell -- NOT a native cmd.exe/PowerShell prompt):
#     pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-qt5-base \
#               mingw-w64-x86_64-pkgconf make
#     Then build from the "MSYS2 MinGW64" shell shortcut (not the plain
#     MSYS2 shell), so `g++`/`pkg-config` resolve to the mingw-w64 versions
#     with Qt5 already on their search path.
#
# Usage (same on every platform, from the appropriate shell above):
#   make                            # build ./noddy (or noddy.exe / noddy.app)
#   make clean
# ============================================================================

CXX      := g++
CC       := gcc

# ---- Source and Object Files -----------------------------------------------
# Automatically finds all .c and .cpp files in the root and qt_compat directories
SRCS_C   := $(wildcard *.c) $(wildcard qt_compat/*.c)
SRCS_CXX := $(wildcard qt_compat/*.cpp)

OBJS     := $(SRCS_C:.c=.o) $(SRCS_CXX:.cpp=.o)

# ---- Platform detection ----------------------------------------------------
# $(OS) is set to "Windows_NT" by the environment on Windows regardless of
# shell (cmd.exe, PowerShell, or an MSYS2/MinGW shell) -- reliable even
# though `uname` also works from an MSYS2 shell, since it's a plain env var
# lookup with no subshell needed. Everywhere else, ask `uname -s` to tell
# Linux and macOS (Darwin) apart.
# [Qt port fix] todo.txt #64 follow-up: $(OS) isn't always inherited (found
# via an automation harness that invokes MSYS2 bash.exe directly rather
# than through a real interactive MSYS2 MinGW64 shell/terminal -- $(OS) came
# back empty there even on a genuine Windows/MinGW64 build, silently
# misdetecting it as Linux, which skipped the Windows-only resource-icon
# build (todo.txt #64) and the -mthreads/-fPIC adjustments). Since MinGW's
# own `uname -s` always reports a "MINGW64_NT-..."/"MSYS_NT-..."-style
# string (never plain "Linux"), checking for a "MINGW" or "MSYS" prefix as
# a fallback catches this even when $(OS) is missing, with no change in
# behavior for real cmd.exe/PowerShell/interactive-MSYS2-shell builds
# (where $(OS) is already set and this branch is never reached).
ifeq ($(OS),Windows_NT)
    NODDY_PLATFORM := Windows
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Darwin)
        NODDY_PLATFORM := macOS
    else ifneq (,$(findstring MINGW,$(UNAME_S)))
        NODDY_PLATFORM := Windows
    else ifneq (,$(findstring MSYS,$(UNAME_S)))
        NODDY_PLATFORM := Windows
    else
        NODDY_PLATFORM := Linux
    endif
endif

# ---- Executable name -------------------------------------------------------
ifeq ($(NODDY_PLATFORM),Windows)
    TARGET := noddy.exe
else ifeq ($(NODDY_PLATFORM),macOS)
    # On macOS, the final output binary must sit deep inside a .app bundle folder structure
    BUNDLE := noddy.app
    TARGET := $(BUNDLE)/Contents/MacOS/noddy
else
    TARGET := noddy
endif


# ---- Qt5 discovery ---------------------------------------------------------
# All three platforms use pkg-config to find Qt5's include/lib flags -- the
# only difference is macOS Homebrew's Qt5 formula is "keg-only" (deliberately
# not linked onto the default pkg-config search path, since Homebrew doesn't
# want to presume you want THIS Qt version as the system default), so its
# .pc files need to be added to PKG_CONFIG_PATH explicitly. Probe both the
# Apple Silicon (/opt/homebrew) and Intel (/usr/local) Homebrew prefixes --
# whichever exists (or neither, on Linux/Windows) is harmless to add.
ifeq ($(NODDY_PLATFORM),macOS)
    export PKG_CONFIG_PATH := $(PKG_CONFIG_PATH):/opt/homebrew/opt/qt@5/lib/pkgconfig:/usr/local/opt/qt@5/lib/pkgconfig
endif

QT_CFLAGS := $(shell pkg-config --cflags Qt5Widgets Qt5Gui Qt5Core Qt5PrintSupport)
QT_LIBS   := $(shell pkg-config --libs Qt5Widgets Qt5Gui Qt5Core Qt5PrintSupport)

CXXFLAGS := -std=c++17 -fPIC -Wall -Wextra -Wno-unused-parameter -g -O2 -Iqt_compat -I. $(QT_CFLAGS)
# -Wno-implicit-function-declaration: this ~30-year-old codebase has always
# relied on plain-C's old implicit-int-return-type behaviour for a handful
# of missing standard-library includes here and there (isdigit/isspace
# without ctype.h, etc) -- harmless on the functions actually affected
# (all int-returning, matching the implicit default) and previously only a
# WARNING under Linux's older default GCC (Ubuntu 20.04, ~9.x). Newer GCC
# (this MinGW toolchain, 14+) promotes it to a hard error by default,
# which would otherwise block a Windows build entirely over pre-existing,
# functionally-harmless omissions unrelated to the Qt port itself. Keeps
# behaviour consistent across platforms rather than papering over it
# per-platform.
# -Wno-implicit-int: same story as -Wno-implicit-function-declaration above
# -- functions DEFINED without an explicit return type (implicit int) were
# also just a warning under Linux's older default GCC, a hard error under
# this newer MinGW one (e.g. EpsilTet.c's `EpsilonCode(...)  { ... }`, no
# leading `int`).
CFLAGS   := -std=gnu17 -fPIC -Wall -Wno-implicit-function-declaration -Wno-implicit-int -g -O2 -Iqt_compat
LDFLAGS  := $(QT_LIBS)

ifeq ($(NODDY_PLATFORM),macOS)
    # Recent clang (Apple's included) promoted several classic K&R-era C
    # patterns from warnings to hard errors by default: calling a function
    # with no visible prototype, a function definition with no return type
    # (implicitly int), and passing/returning an int where a pointer is
    # expected (or vice versa) without a cast. This 1990s/2000s codebase is
    # full of all three, and they're truly harmless here -- everything
    # still links and runs correctly, clang is just no longer willing to
    # silently accept it. Downgrading them back to warnings (their
    # behavior on Linux gcc, and on older clang) unblocks the build without
    # touching decades-old source files.
    CFLAGS += -Wno-error=implicit-function-declaration -Wno-error=implicit-int -Wno-error=int-conversion
    # There's no system <malloc.h> on macOS at all, unlike Linux. However,
    # this project already ships its OWN malloc.h alongside the .c files
    # (clang confirms this: "file not found with <angled> include; use
    # quotes instead" only happens when quote-form lookup -- which always
    # checks the including file's own directory -- finds a match that
    # angle-bracket lookup, which skips that directory, does not). Adding
    # the current directory to the include path lets `#include <malloc.h>`
    # find that local header on macOS too, without touching any .c files.
    # Scoped to macOS only so Linux/Windows keep resolving <malloc.h> to
    # their own system header as before.
    CFLAGS += -I.
endif

# -fPIC is meaningless (and warns) when targeting Windows PE via MinGW --
# every object is position-independent there by construction, there's no
# separate "PIC" relocation mode to opt into the way there is on ELF/Mach-O.
ifeq ($(NODDY_PLATFORM),Windows)
    CXXFLAGS := $(filter-out -fPIC,$(CXXFLAGS))
    CFLAGS   := $(filter-out -fPIC,$(CFLAGS))
endif

# [Qt port fix] todo.txt #64: noddy_win.rc (gives noddy.exe its application
# icon, see that file) existed in the tree but was never actually wired
# into a build rule here -- windres was never invoked and nothing was
# linked in, so Windows builds silently produced an exe with no custom
# icon at all. Windows-only, since windres/.rc resources aren't a thing
# on Linux/macOS.
ifeq ($(NODDY_PLATFORM),Windows)
    OBJS += noddy_win.o
endif

%.o: %.rc
	windres $< -o $@

# ---- Build Recipes ---------------------------------------------------------

.PHONY: all clean

all: $(TARGET)
ifeq ($(NODDY_PLATFORM),macOS)
	@echo "Creating macOS Bundle Structure and Info.plist..."
	@mkdir -p $(BUNDLE)/Contents
	@echo '<?xml version="1.0" encoding="UTF-8"?>' > $(BUNDLE)/Contents/Info.plist
	@echo '<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://apple.com">' >> $(BUNDLE)/Contents/Info.plist
	@echo '<plist version="1.0"><dict>' >> $(BUNDLE)/Contents/Info.plist
	@echo '  <key>CFBundleExecutable</key><string>noddy</string>' >> $(BUNDLE)/Contents/Info.plist
	@echo '  <key>CFBundleIdentifier</key><string>com.yourcompany.noddy</string>' >> $(BUNDLE)/Contents/Info.plist
	@echo '  <key>CFBundleName</key><string>Noddy</string>' >> $(BUNDLE)/Contents/Info.plist
	@echo '  <key>CFBundlePackageType</key><string>APPL</string>' >> $(BUNDLE)/Contents/Info.plist
	@echo '  <key>CFBundleShortVersionString</key><string>1.0</string>' >> $(BUNDLE)/Contents/Info.plist
	@echo '  <key>NSPrincipalClass</key><string>NSApplication</string>' >> $(BUNDLE)/Contents/Info.plist
	@echo '</dict></plist>' >> $(BUNDLE)/Contents/Info.plist
endif

$(TARGET): $(OBJS)
	@mkdir -p $(dir $(TARGET))
	$(CXX) -o $@ $(OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@echo "Cleaning up build artifacts..."
	rm -f *.o
	rm -f qt_compat/*.o
	rm -f noddy noddy.exe
	rm -rf noddy.app