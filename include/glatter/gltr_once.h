#pragma once

#ifdef _WIN32
  #include <windows.h>
  typedef INIT_ONCE gltr_once_t;
  #define GLTR_ONCE_INIT INIT_ONCE_STATIC_INIT
  static BOOL CALLBACK gltr__once_adapter(PINIT_ONCE init_once, PVOID param, PVOID *context) {
      (void)init_once;
      (void)context;
      ((void(*)(void))param)();
      return TRUE;
  }
  static inline void gltr_call_once(gltr_once_t *o, void (*fn)(void)) {
      InitOnceExecuteOnce(o, gltr__once_adapter, (PVOID)fn, NULL);
  }
#else
  #include <pthread.h>
  typedef pthread_once_t gltr_once_t;
  #define GLTR_ONCE_INIT PTHREAD_ONCE_INIT
  static inline void gltr_call_once(gltr_once_t *o, void (*fn)(void)) {
      pthread_once(o, fn);
  }
#endif
