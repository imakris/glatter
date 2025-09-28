#pragma once

/* Tiny atomic helpers used for first-use resolution across C++/C toolchains.
 * Also provides GLATTER_ATOMIC_INIT_PTR(x) to portably initialize atomic
 * variables holding function pointers (esp. MSVC C Interlocked branch).
 */

#if defined(__cplusplus)

/* --- C++11+: use std::atomic --- */
#  include <atomic>
#  define glatter_atomic(T)          std::atomic<T>
#  define GLATTER_ATOMIC_LOAD(a)     (a.load(std::memory_order_acquire))
#  define GLATTER_ATOMIC_STORE(a,v)  (a.store((v), std::memory_order_release))
#  define GLATTER_ATOMIC_CAS(a,exp,des) \
      (a.compare_exchange_strong((exp),(des), std::memory_order_acq_rel, std::memory_order_acquire))
#  define GLATTER_ATOMIC_INIT_PTR(x) (x)

#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L && !defined(__STDC_NO_ATOMICS__)

/* --- C11 atomics --- */
#  include <stdatomic.h>
#  define glatter_atomic(T)          _Atomic(T)
#  define GLATTER_ATOMIC_LOAD(a)     atomic_load_explicit(&(a), memory_order_acquire)
#  define GLATTER_ATOMIC_STORE(a,v)  atomic_store_explicit(&(a), (v), memory_order_release)
#  define GLATTER_ATOMIC_CAS(a,exp,des) \
      atomic_compare_exchange_strong_explicit(&(a), &(exp), (des), memory_order_acq_rel, memory_order_acquire)
#  define GLATTER_ATOMIC_INIT_PTR(x) (x)

#elif defined(_WIN32)

/* --- MSVC C (no C11): use Interlocked* on PVOID --- */
#  include <windows.h>
#  include <stdint.h>
   typedef PVOID glatter_atomic_voidp;
#  define GLATTER_ATOMIC_CAST_PTR(v) ((PVOID)(uintptr_t)(v))
#  define glatter_atomic(T)          glatter_atomic_voidp
#  define GLATTER_ATOMIC_LOAD(a)     (InterlockedCompareExchangePointer(&(a), NULL, NULL))
#  define GLATTER_ATOMIC_STORE(a,v)  ((void)InterlockedExchangePointer(&(a), GLATTER_ATOMIC_CAST_PTR(v)))
#  define GLATTER_ATOMIC_CAS(a,exp,des) \
      (InterlockedCompareExchangePointer(&(a), GLATTER_ATOMIC_CAST_PTR(des), GLATTER_ATOMIC_CAST_PTR(exp)) == GLATTER_ATOMIC_CAST_PTR(exp))
#  define GLATTER_ATOMIC_INIT_PTR(x) GLATTER_ATOMIC_CAST_PTR(x)

#else

/* --- GCC/Clang C builtins --- */
#  define glatter_atomic(T)          T
#  define GLATTER_ATOMIC_LOAD(a)     __atomic_load_n(&(a), __ATOMIC_ACQUIRE)
#  define GLATTER_ATOMIC_STORE(a,v)  __atomic_store_n(&(a),(v), __ATOMIC_RELEASE)
#  define GLATTER_ATOMIC_CAS(a,exp,des) \
      __atomic_compare_exchange_n(&(a), &(exp), (des), 0, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE)
#  define GLATTER_ATOMIC_INIT_PTR(x) (x)

#endif
