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

#if defined(_WIN32)
INIT_ONCE glatter_thread_once = INIT_ONCE_STATIC_INIT;
DWORD     glatter_thread_id   = 0;
#elif defined(__APPLE__) || defined(__unix__)
pthread_once_t glatter_thread_once = PTHREAD_ONCE_INIT;
pthread_t      glatter_thread_id;
#else
#error "Unsupported platform"
#endif

int glatter_owner_bound_explicitly = 0;
