#pragma once

/* glatter_solo.h
 * Header-only C++ convenience wrapper.
 * Defines GLATTER_HEADER_ONLY and includes glatter.h.
 *
 * NOTE 1: Do not mix header-only and compiled-TU modes in the same binary.
 * If you compile glatter.c (compiled mode), do NOT include this header anywhere.
 *
 * NOTE 2: In header-only builds the tiny loader state lives per translation unit.
 * Detection is deterministic (e.g., WGL?EGL on Windows; GLX?EGL on POSIX), so
 * all TUs converge to the same WSI under the same build and environment.
 * You can still use the compiled C translation unit variant if you prefer having
 * one shared state for the entire process or slightly smaller binaries,
 * but not for correctness.
 */

#if !defined(__cplusplus)
#  error "glatter_solo.h is C++-only (header-only mode). Use glatter.h with glatter.c for C."
#endif

/* Tag this TU as header-only. */
#ifndef GLATTER_HEADER_ONLY
#  define GLATTER_HEADER_ONLY 1
#endif

#if defined(_MSC_VER)
#pragma detect_mismatch("glatter-build-mode", "header")
#endif

#ifndef GLATTER_USER_CONFIGURED
#  define GLATTER_USER_CONFIGURED 0
#endif

#include <glatter/glatter.h>
