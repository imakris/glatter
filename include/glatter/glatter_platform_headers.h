//NEW 1

#ifndef GLATTER_PLATFORM_HEADERS_H_DEFINED
#define GLATTER_PLATFORM_HEADERS_H_DEFINED

// When introducing a new platform, this file needs to be modified.

// WARNING: This file is processed by glatter.py, which makes certain
// assumptions about its format.
// A platform's header set must be contained within a single #if/ifdef block which
// begins with #define GLATTER_PLATFORM_DIR the_name_of_the_platform_directory
// It is OK to have other/irrelevant statements inside that block.

/* -----------------------------------------------------------
   Platform selection
   One of the following platform macros must be defined
   in glatter_config.h:
     - GLATTER_WINDOWS_WGL_GL
     - GLATTER_MESA_GLX_GL
     - GLATTER_MESA_EGL_GLES
   ----------------------------------------------------------- */

#if defined(GLATTER_WINDOWS_WGL_GL)

/* ---------------- Windows: WGL + desktop GL ---------------- */
#define GLATTER_PLATFORM_DIR glatter_windows_wgl_gl

/* Windows macros (WINGDIAPI, APIENTRY) come from <windows.h> */
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

/* Core desktop GL for Windows */
#include "headers/windows_gl_basic/GL.h"

/* Khronos extension headers for GL and WGL */
#include "headers/khronos_gl/glext.h"
#include "headers/khronos_gl/wglext.h"

/* Optional GLU */
#if defined(GLATTER_GLU)
#include "headers/windows_gl_basic/GLU.h"
#endif

/* Sanity: no incompatible wrappers with this platform */
#if defined(GLATTER_GLX) || defined(GLATTER_EGL)
#error One of the wrappers defined is not relevant to the selected platform. Please review your glatter_config.h.
#endif

#elif defined(GLATTER_MESA_GLX_GL)

/* ---------------- Linux/BSD: GLX + desktop GL -------------- */
#define GLATTER_PLATFORM_DIR glatter_mesa_glx_gl

/* Core desktop GL for Mesa */
#include "headers/mesa_gl_basic/GL/gl.h"

/* GLX core + extensions */
#include "headers/mesa_gl_basic/GL/glx.h"
#include "headers/khronos_gl/glxext.h"

/* Desktop GL extensions */
#include "headers/khronos_gl/glext.h"

/* Optional GLU */
#if defined(GLATTER_GLU)
#include "headers/mesa_gl_basic/GL/glu.h"
#endif

/* Sanity: no incompatible wrappers with this platform */
#if defined(GLATTER_WGL) || defined(GLATTER_EGL)
#error One of the wrappers defined is not relevant to the selected platform. Please review your glatter_config.h.
#endif

#elif defined(GLATTER_MESA_EGL_GLES)

/* ---------------- Any: EGL + OpenGL ES --------------------- */
#define GLATTER_PLATFORM_DIR glatter_mesa_egl_gles

/* EGL core + extensions */
#include "headers/khronos_egl/egl.h"
#include "headers/khronos_egl/eglext.h"

/* GLES2 core + extensions (adjust if you also ship GLES1/3) */
#include "headers/khronos_gles2/gles2.h"
#include "headers/khronos_gles2/gles2ext.h"

/* Sanity: no incompatible wrappers with this platform */
#if defined(GLATTER_WGL) || defined(GLATTER_GLX)
#error One of the wrappers defined is not relevant to the selected platform. Please review your glatter_config.h.
#endif

#else
#error No supported platform selected. Define one of GLATTER_WINDOWS_WGL_GL, GLATTER_MESA_GLX_GL, or GLATTER_MESA_EGL_GLES in glatter_config.h.
#endif /* platform selection */

/* -----------------------------------------------------------
   Printf format fallbacks for 64-bit integers
   glatter.h includes <inttypes.h> before this file. If PRId64
   (and friends) are still not defined (e.g., older MSVC),
   provide sane defaults here.
   ----------------------------------------------------------- */
#ifndef PRId64
  #ifdef _MSC_VER
    #define PRId64 "I64d"
    #define PRIu64 "I64u"
    #define PRIx64 "I64x"
  #else
    #define PRId64 "lld"
    #define PRIu64 "llu"
    #define PRIx64 "llx"
  #endif
#endif

/* -----------------------------------------------------------
   Enum type aliases used by enum_to_string_* helpers
   These are always available, independent of platform branch.
   You can tighten them later (e.g., EGLint for EGL) and regenerate.
   ----------------------------------------------------------- */
#ifdef GLATTER_GL
typedef GLenum GLATTER_ENUM_GL;
#else
typedef unsigned int GLATTER_ENUM_GL;
#endif

typedef unsigned int GLATTER_ENUM_GLX;
typedef unsigned int GLATTER_ENUM_WGL;

#ifdef GLATTER_GLU
typedef GLUenum GLATTER_ENUM_GLU;
#else
typedef unsigned int GLATTER_ENUM_GLU;
#endif

#ifdef GLATTER_EGL
typedef EGLint GLATTER_ENUM_EGL;
#else
typedef int GLATTER_ENUM_EGL;
#endif

#endif /* GLATTER_PLATFORM_HEADERS_H_DEFINED */
