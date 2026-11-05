/* Shim for macOS builds only. The original 1990s/2000s Noddy sources
 * include <malloc.h>, a Linux/glibc-specific header that doesn't exist
 * anywhere on macOS (not even in the SDK) -- unlike Linux, where the
 * system provides a real one. macOS instead exposes malloc()/free()/etc.
 * via <stdlib.h>, so this shim just forwards there.
 *
 * This file must sit in the SAME DIRECTORY as the .c files (next to
 * Makefile.qt, 3dHier.c, allSurf.c, etc.) -- Makefile.qt's macOS branch
 * adds "-I." to CFLAGS, which makes the compiler look here, in the
 * current directory, before falling through to the (nonexistent) system
 * malloc.h. On Linux/Windows this file is simply never reached, since
 * the real system <malloc.h> is found first there.
 */
#include <stdlib.h>
