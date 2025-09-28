/* glatter_solo.h
 * Header-only C++ convenience wrapper.
 * Defines GLATTER_HEADER_ONLY and includes glatter.h.
 *
 * Note: In header-only builds the tiny loader state lives per translation unit.
 * Detection is deterministic (e.g., WGL→EGL on Windows; GLX→EGL on POSIX), so
 * all TUs converge to the same WSI under the same build and environment.
 * You can still use the compiled C translation unit variant if you prefer having
 * one shared state for the entire process or slightly smaller binaries,
 * but not for correctness.
 */

#ifndef GLATTER_SOLO_H
#define GLATTER_SOLO_H

/* Enable header-only mode explicitly for C++ consumers. */
#if defined(__cplusplus) && !defined(GLATTER_HEADER_ONLY)
#  define GLATTER_HEADER_ONLY 1
#endif

/* Mark that the default configuration should be used. */
#ifndef GLATTER_USER_CONFIGURED
#  define GLATTER_USER_CONFIGURED 0
#endif

#include <glatter/glatter.h>

#endif /* GLATTER_SOLO_H */
