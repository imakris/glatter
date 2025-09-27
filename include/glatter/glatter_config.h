#ifndef GLATTER_CONFIG_H_DEFINED
#define GLATTER_CONFIG_H_DEFINED


/* =========================
   Zero-config defaults
   ========================= */

/* If the user didn't configure Glatter manually, pick sensible defaults. */
#if !defined(GLATTER_USER_CONFIGURED) || (GLATTER_USER_CONFIGURED == 0)

    /* Core GL wrappers are available unless explicitly disabled. */
    #ifndef GLATTER_GL
        #define GLATTER_GL 1
    #endif

    /* Platform defaults based on the host toolchain. */
    #if defined(_WIN32)
        #ifndef GLATTER_WINDOWS_WGL_GL
            #define GLATTER_WINDOWS_WGL_GL 1
        #endif
        #ifndef GLATTER_WGL
            #define GLATTER_WGL 1
        #endif
    #elif defined(__ANDROID__) || defined(__EMSCRIPTEN__)
        #ifndef GLATTER_MESA_EGL_GLES
            #define GLATTER_MESA_EGL_GLES 1
        #endif
        #ifndef GLATTER_EGL
            #define GLATTER_EGL 1
        #endif
        /* Default to the highest GLES profile shipped with the headers. */
        #ifndef GLATTER_EGL_GLES_3_2
            #define GLATTER_EGL_GLES_3_2 1
        #endif
    #else
        #ifndef GLATTER_MESA_GLX_GL
            #define GLATTER_MESA_GLX_GL 1
        #endif
        #ifndef GLATTER_GLX
            #define GLATTER_GLX 1
        #endif
    #endif

#endif /* !defined(GLATTER_USER_CONFIGURED) || (GLATTER_USER_CONFIGURED == 0) */


#ifndef GLATTER_WSI_AUTO_VALUE
#define GLATTER_WSI_AUTO_VALUE 0
#define GLATTER_WSI_WGL_VALUE  1
#define GLATTER_WSI_GLX_VALUE  2
#define GLATTER_WSI_EGL_VALUE  3
#endif


/////////////////////////////////
// Explicit platform selection //
/////////////////////////////////
//
// NOTE: For GLES the platform must be specified explicitly.
//
// #define GLATTER_WINDOWS_WGL_GL
// #define GLATTER_MESA_GLX_GL
// #define GLATTER_MESA_EGL_GLES
// #define GLATTER_EGL_GLES_1_1
// #define GLATTER_EGL_GLES2_2_0
// #define GLATTER_EGL_GLES_3_0
// #define GLATTER_EGL_GLES_3_1
// #define GLATTER_EGL_GLES_3_2

// If no platform is defined, it will be set according to the operating system.
#if !defined(GLATTER_WINDOWS_WGL_GL) &&\
    !defined(GLATTER_MESA_GLX_GL)    &&\
    !defined(GLATTER_MESA_EGL_GLES)  &&\
    !defined(GLATTER_EGL_GLES_1_1)   &&\
    !defined(GLATTER_EGL_GLES2_2_0)  &&\
    !defined(GLATTER_EGL_GLES_3_0)   &&\
    !defined(GLATTER_EGL_GLES_3_1)   &&\
    !defined(GLATTER_EGL_GLES_3_2)

#if defined(_WIN32)
    #define GLATTER_WINDOWS_WGL_GL 1
#elif defined(__linux__)
    #define GLATTER_MESA_GLX_GL 1
#endif

#endif


#if defined(GLATTER_EGL_GLES_1_1)  ||\
    defined(GLATTER_EGL_GLES2_2_0) ||\
    defined(GLATTER_EGL_GLES_3_0)  ||\
    defined(GLATTER_EGL_GLES_3_1)  ||\
    defined(GLATTER_EGL_GLES_3_2)

    #ifndef GLATTER_MESA_EGL_GLES
        #define GLATTER_MESA_EGL_GLES 1
    #endif

#endif


////////////////////////////////
// Explicit wrapper selection //
////////////////////////////////
// #define GLATTER_GL
// #define GLATTER_EGL
// #define GLATTER_WGL
// #define GLATTER_GLX
// #define GLATTER_GLU

// If no wrapper is defined, GL is set, and one of ELG/GLX/WGL depending on the platform
#if !defined(GLATTER_GL) && !defined(GLATTER_EGL) && !defined(GLATTER_WGL) &&\
    !defined(GLATTER_GLX) && !defined(GLATTER_GLU)

    #define GLATTER_GL 1
    #if defined(GLATTER_WINDOWS_WGL_GL)
        #define GLATTER_WGL 1
    #elif defined (GLATTER_MESA_GLX_GL)
        #define GLATTER_GLX 1
    #else // GLES
        #define GLATTER_EGL 1
    #endif

    // #define GLATTER_GLU  // GLU is not enabled by default

#endif



//////////////////////////////////////
// Debugging functionality switches //
//////////////////////////////////////
// #define GLATTER_LOG_ERRORS
// #define GLATTER_LOG_CALLS

// Unless specified otherwise, GL errors will be logged in debug builds
#if !defined(GLATTER_LOG_ERRORS) && !defined(GLATTER_LOG_CALLS)
    #ifndef NDEBUG
        #define GLATTER_LOG_ERRORS 1
    #endif
#endif



///////////////////////////////////////////////////
// X error handler switch (only relevant to GLX) //
///////////////////////////////////////////////////
// Installing the GLATTER X error handler replaces the process-wide Xlib
// handler. Define GLATTER_DO_NOT_INSTALL_X_ERROR_HANDLER to opt out and
// install your own (for example after calling XInitThreads()).
//#define GLATTER_DO_NOT_INSTALL_X_ERROR_HANDLER

////////////////////////////////////////////////////////
// Thread ownership enforcement (header-only C++) switch //
////////////////////////////////////////////////////////
// Define GLATTER_REQUIRE_EXPLICIT_OWNER_BIND to disable the automatic
// owner-thread binding performed the first time a wrapped call executes in
// header-only builds. When set, applications must call
// glatter_bind_owner_to_current_thread() on the intended render thread before
// making GL calls; otherwise the library aborts to signal the configuration
// error.
//#define GLATTER_REQUIRE_EXPLICIT_OWNER_BIND

/////////////////////////////////////
// Windows character encoding switch //
/////////////////////////////////////
// Define GLATTER_WINDOWS_MBCS to treat TCHAR as CHAR when generating bindings
// for non-UNICODE builds. The generator assumes UNICODE (TCHAR -> WCHAR) by
// default.
#if defined(_WIN32)
# if !defined(UNICODE) && !defined(_UNICODE)
#   ifndef GLATTER_WINDOWS_MBCS
#     define GLATTER_WINDOWS_MBCS 1
#   endif
# endif
#endif


#endif
