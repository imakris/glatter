#ifndef __GLATTER_SYSTEM_HEADERS_H__
#define __GLATTER_SYSTEM_HEADERS_H__

// This file should be modified, to get the desired system headers included.

#if !defined(GLATTER_GLES)

#if defined(_WIN32)

#include <windows.h>
#include <GL/gl.h>
#include "../../tools/glatter/input_headers/glext/wglext.h"

#elif defined (__linux__)

#include <GL/gl.h>
#include <GL/glx.h>
#include "../../tools/glatter/input_headers/glext/glxext.h"

#include <pthread.h>
#include <dlfcn.h>

#endif

#include "../../tools/glatter/input_headers/glext/glext.h"

#else //defined(GLATTER_GLES)

#include <GLES/gl.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

#endif


#endif