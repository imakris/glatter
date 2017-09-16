#ifndef __GLATTER_CONFIG_H__
#define __GLATTER_CONFIG_H__

#define GLATTER_GL
//#define GLATTER_GLES

#if defined(GLATTER_GLES)
    #define GLATTER_EGL
#elif defined(_WIN32)
    #define GLATTER_WGL
#elif defined(__linux__)
    #define GLATTER_GLX
    //#define GLATTER_DO_NOT_INSTALL_X_ERROR_HANDLER
#endif

//#define GLATTER_GLU

#ifndef NDEBUG
#define GLATTER_LOG_ERRORS
//#define GLATTER_LOG_CALLS
#endif

//#define GLATTER_HEADER_ONLY

#endif
