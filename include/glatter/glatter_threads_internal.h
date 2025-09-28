/* glatter_threads_internal.h (private)
   Threading helpers for first-use resolution.
   Policy:
     - If C++11 or C11 atomics exist => use them.
     - Else => use pthread_once / InitOnce.
   No user knobs here; keep glatter_config.h user-facing and clean.
*/
#pragma once

/* Optional hard opt-out for niche/embedded builds.
   Define via compiler flags only if you *guarantee* single-threaded init:
   -DGLATTER_ASSUME_SINGLE_THREADED=1
*/
#if defined(GLATTER_ASSUME_SINGLE_THREADED) && GLATTER_ASSUME_SINGLE_THREADED
#  define GLATTER_INTERNAL_SINGLE_THREADED 1
#else
#  define GLATTER_INTERNAL_SINGLE_THREADED 0
#endif

/* ---- Atomics detection ---- */
#if !GLATTER_INTERNAL_SINGLE_THREADED
#  if defined(__cplusplus) && __cplusplus >= 201103L
#    define GLATTER_HAVE_CXX11_ATOMICS 1
#  else
#    define GLATTER_HAVE_CXX11_ATOMICS 0
#  endif
#  if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L && !defined(__STDC_NO_ATOMICS__)
#    define GLATTER_HAVE_C11_ATOMICS 1
#  else
#    define GLATTER_HAVE_C11_ATOMICS 0
#  endif
#else
#  define GLATTER_HAVE_CXX11_ATOMICS 0
#  define GLATTER_HAVE_C11_ATOMICS   0
#endif

#define GLATTER_USE_ATOMICS (!GLATTER_INTERNAL_SINGLE_THREADED && (GLATTER_HAVE_CXX11_ATOMICS || GLATTER_HAVE_C11_ATOMICS))

/* ---- Atomic pointer wrappers (or plain pointers) ---- */
#if GLATTER_USE_ATOMICS
#  if GLATTER_HAVE_CXX11_ATOMICS
#    include <atomic>
#    define GLATTER_ATOMIC(T)                std::atomic<T>
#    define GLATTER_ATOMIC_LOCAL_INIT(x)     { x }
#    define GLATTER_ATOMIC_LOAD(v)           (v).load(std::memory_order_acquire)
#    define GLATTER_ATOMIC_STORE(v,x)        (v).store((x), std::memory_order_release)
#  else /* C11 */
#    include <stdatomic.h>
#    define GLATTER_ATOMIC(T)                _Atomic(T)
#    define GLATTER_ATOMIC_LOCAL_INIT(x)     = (x)
#    define GLATTER_ATOMIC_LOAD(v)           atomic_load_explicit(&(v), memory_order_acquire)
#    define GLATTER_ATOMIC_STORE(v,x)        atomic_store_explicit(&(v), (x), memory_order_release)
#  endif
#else
#  define GLATTER_ATOMIC(T)                  T
#  define GLATTER_ATOMIC_LOCAL_INIT(x)       = (x)
#  define GLATTER_ATOMIC_LOAD(v)             (v)
#  define GLATTER_ATOMIC_STORE(v,x)          ((v) = (x))
#endif

/* ---- Once helpers for non-atomic path ---- */
#if !GLATTER_USE_ATOMICS && !GLATTER_INTERNAL_SINGLE_THREADED
#  ifdef _WIN32
#    include <windows.h>
     /* Per-TU once tokens are fine (header-only creates per-TU inlines). */
#    define GLATTER_ONCE_TYPE               INIT_ONCE
#    define GLATTER_ONCE_INIT               INIT_ONCE_STATIC_INIT
#    define GLATTER_ONCE(runonce_fn, token) InitOnceExecuteOnce(&(token), (runonce_fn), NULL, NULL)
#    define GLATTER_ONCE_CB(name)           static BOOL CALLBACK name(PINIT_ONCE init_once_arg, PVOID parameter_arg, PVOID* context_arg)
#    define GLATTER_ONCE_CB_UNUSED()        (void)init_once_arg; (void)parameter_arg; (void)context_arg
#    define GLATTER_ONCE_RETURN(val)        return (val)
#  else
#    include <pthread.h>
#    define GLATTER_ONCE_TYPE               pthread_once_t
#    define GLATTER_ONCE_INIT               PTHREAD_ONCE_INIT
#    define GLATTER_ONCE(runonce_fn, token) pthread_once(&(token), (runonce_fn))
#    define GLATTER_ONCE_CB(name)           static void name(void)
#    define GLATTER_ONCE_CB_UNUSED()        (void)0
#    define GLATTER_ONCE_RETURN(val)        ((void)(val))
#  endif
#else
   /* No once needed (atomics or single-threaded). */
#  define GLATTER_ONCE_TYPE                 int
#  define GLATTER_ONCE_INIT                 0
#  define GLATTER_ONCE(f, t)                ((void)0)
#  define GLATTER_ONCE_CB(name)             static void name(void)
#  define GLATTER_ONCE_CB_UNUSED()          (void)0
#  define GLATTER_ONCE_RETURN(val)          ((void)0)
#endif

#if !GLATTER_USE_ATOMICS && !GLATTER_INTERNAL_SINGLE_THREADED
#  ifdef _WIN32
#    define GLATTER_ONCE_CB_TYPE BOOL (CALLBACK *)(PINIT_ONCE, PVOID, PVOID*)
#  else
#    define GLATTER_ONCE_CB_TYPE void (*)(void)
#  endif
#else
#  define GLATTER_ONCE_CB_TYPE void (*)(void)
#endif

#ifndef GLATTER_ONCE_CB_UNUSED
#  define GLATTER_ONCE_CB_UNUSED()          (void)0
#endif
