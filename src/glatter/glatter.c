/* Always include Windows/POSIX system headers BEFORE any GL headers. */
#include <glatter/glatter_config.h>

#ifdef GLATTER_HEADER_ONLY
#error "Do not compile glatter.c when GLATTER_HEADER_ONLY is defined."
#endif

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#elif defined(__APPLE__) || defined(__unix__)
#include <pthread.h>
#endif

/* This will include platform headers in the correct, internal order. */
#include <glatter/glatter_def.h>

/* C/C++ compiled mode: globals are deliberate.
 * Header-only C++ uses function-local statics; compiled mode centralizes state
 * to one TU for consistent linkage and smaller object size.
 */
#if defined(_WIN32)
INIT_ONCE glatter_thread_once = INIT_ONCE_STATIC_INIT;
DWORD     glatter_thread_id   = 0;
int       glatter_owner_bound_explicitly = 0;
int       glatter_owner_thread_initialized = 0;
#elif defined(__APPLE__) || defined(__unix__)
pthread_once_t glatter_thread_once = PTHREAD_ONCE_INIT;
pthread_t      glatter_thread_id;
int            glatter_owner_bound_explicitly = 0;
int            glatter_owner_thread_initialized = 0;
#else
#error "Unsupported platform"
#endif

#if defined(__GNUC__)
__attribute__((visibility("default")))
#endif
const int glatter_build_mode_separate = 1;
/* Link-time guard: fails the build if both header-only and separate-build are linked. */
