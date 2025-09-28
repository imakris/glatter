#ifndef GLATTER_CONFIG_INTERNAL_H
#define GLATTER_CONFIG_INTERNAL_H

/* ------------------------------------------------------------------------- */
/* 1) Legacy compatibility: map old-style toggles onto the new enums/flags.  */
/* ------------------------------------------------------------------------- */

/* Legacy platform selectors imply the new GLATTER_PLATFORM value. */
#if defined(GLATTER_WINDOWS_WGL_GL)
#  undef GLATTER_PLATFORM
#  define GLATTER_PLATFORM GLATTER_PLATFORM_WGL
#endif
#if defined(GLATTER_MESA_GLX_GL)
#  undef GLATTER_PLATFORM
#  define GLATTER_PLATFORM GLATTER_PLATFORM_GLX
#endif
#if defined(GLATTER_MESA_EGL_GLES)
#  undef GLATTER_PLATFORM
#  define GLATTER_PLATFORM GLATTER_PLATFORM_EGL
#endif

/* Legacy GLES profiles imply the consolidated GLATTER_GLES_VERSION knob. */
#if defined(GLATTER_EGL_GLES_1_1)
#  undef GLATTER_GLES_VERSION
#  define GLATTER_GLES_VERSION 11
#endif
#if defined(GLATTER_EGL_GLES2_2_0)
#  undef GLATTER_GLES_VERSION
#  define GLATTER_GLES_VERSION 20
#endif
#if defined(GLATTER_EGL_GLES_3_0)
#  undef GLATTER_GLES_VERSION
#  define GLATTER_GLES_VERSION 30
#endif
#if defined(GLATTER_EGL_GLES_3_1)
#  undef GLATTER_GLES_VERSION
#  define GLATTER_GLES_VERSION 31
#endif
#if defined(GLATTER_EGL_GLES_3_2)
#  undef GLATTER_GLES_VERSION
#  define GLATTER_GLES_VERSION 32
#endif

/* ------------------------------------------------------------------------- */
/* 2) Zero-config defaults (if the user did not override the knobs).         */
/* ------------------------------------------------------------------------- */

#ifndef GLATTER_GL
#  define GLATTER_GL 1
#endif
#ifndef GLATTER_GLU
#  define GLATTER_GLU 0
#endif
#ifndef GLATTER_GLES_VERSION
#  define GLATTER_GLES_VERSION 0
#endif
#ifndef GLATTER_PLATFORM
#  define GLATTER_PLATFORM GLATTER_PLATFORM_AUTO
#endif
#ifndef GLATTER_LOG_ERRORS
#  define GLATTER_LOG_ERRORS 1
#endif
#ifndef GLATTER_LOG_CALLS
#  define GLATTER_LOG_CALLS 0
#endif
#ifndef GLATTER_REQUIRE_EXPLICIT_OWNER_BIND
#  define GLATTER_REQUIRE_EXPLICIT_OWNER_BIND 0
#endif
#ifndef GLATTER_INSTALL_X_ERROR_HANDLER
#  define GLATTER_INSTALL_X_ERROR_HANDLER 1
#endif

/* ------------------------------------------------------------------------- */
/* 3) Platform mapping / auto-detect                                         */
/* ------------------------------------------------------------------------- */

#if (GLATTER_PLATFORM == GLATTER_PLATFORM_WGL)
#  define GLATTER_WINDOWS_WGL_GL 1
#  define GLATTER_WGL 1
#elif (GLATTER_PLATFORM == GLATTER_PLATFORM_GLX)
#  define GLATTER_MESA_GLX_GL 1
#  define GLATTER_GLX 1
#elif (GLATTER_PLATFORM == GLATTER_PLATFORM_EGL)
#  define GLATTER_MESA_EGL_GLES 1
#  define GLATTER_EGL 1
#elif (GLATTER_PLATFORM == GLATTER_PLATFORM_AUTO)
   /* Preserve the historical zero-config behaviour. */
#  if defined(_WIN32)
#    define GLATTER_WINDOWS_WGL_GL 1
#    define GLATTER_WGL 1
#  elif defined(__ANDROID__) || defined(__EMSCRIPTEN__)
#    define GLATTER_MESA_EGL_GLES 1
#    define GLATTER_EGL 1
#    if GLATTER_GLES_VERSION == 0
#      undef GLATTER_GLES_VERSION
#      define GLATTER_GLES_VERSION 32
#      define GLATTER_EGL_GLES_3_2 1
#    endif
#  else
#    define GLATTER_MESA_GLX_GL 1
#    define GLATTER_GLX 1
#  endif
#else
#  error "Invalid GLATTER_PLATFORM value"
#endif

/* ------------------------------------------------------------------------- */
/* 4) Derive GLES/EGL feature flags from the consolidated version switch.    */
/* ------------------------------------------------------------------------- */

#if GLATTER_GLES_VERSION
#  define GLATTER_EGL 1
#  if   GLATTER_GLES_VERSION == 11
#    define GLATTER_EGL_GLES_1_1 1
#  elif GLATTER_GLES_VERSION == 20
#    define GLATTER_EGL_GLES2_2_0 1
#  elif GLATTER_GLES_VERSION == 30
#    define GLATTER_EGL_GLES_3_0 1
#  elif GLATTER_GLES_VERSION == 31
#    define GLATTER_EGL_GLES_3_1 1
#  elif GLATTER_GLES_VERSION == 32
#    define GLATTER_EGL_GLES_3_2 1
#  else
#    error "Invalid GLATTER_GLES_VERSION: allowed {0,11,20,30,31,32}"
#  endif
#endif

/* If no API toggles were left enabled, fall back to core GL. */
#if !defined(GLATTER_GL) && !defined(GLATTER_EGL) && !defined(GLATTER_WGL) && \
    !defined(GLATTER_GLX) && !defined(GLATTER_GLU)
#  define GLATTER_GL 1
#  if defined(GLATTER_WINDOWS_WGL_GL)
#    define GLATTER_WGL 1
#  elif defined(GLATTER_MESA_GLX_GL)
#    define GLATTER_GLX 1
#  else
#    define GLATTER_EGL 1
#  endif
#endif

/* ------------------------------------------------------------------------- */
/* 5) Auxiliary compatibility flags.                                          */
/* ------------------------------------------------------------------------- */

#if !GLATTER_INSTALL_X_ERROR_HANDLER
#  define GLATTER_DO_NOT_INSTALL_X_ERROR_HANDLER 1
#endif

#if defined(_WIN32)
#  if !defined(UNICODE) && !defined(_UNICODE)
#    ifndef GLATTER_WINDOWS_MBCS
#      define GLATTER_WINDOWS_MBCS 1
#    endif
#  endif
#endif

#endif /* GLATTER_CONFIG_INTERNAL_H */
