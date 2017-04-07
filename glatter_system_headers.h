#ifndef __GLATTER_SYSTEM_HEADERS_H__
#define __GLATTER_SYSTEM_HEADERS_H__

// This file should be modified, to get the desired system headers included.

#if defined(_WIN32)

#include <windows.h>
#include <GL/gl.h>
#include "input_headers/glext/wglext.h"

#elif defined (__linux__)

#include <GL/gl.h>
#include <GL/glx.h>
#include "input_headers/glext/glxext.h"

#include <pthread.h>
#include <dlfcn.h>


#endif

#include "input_headers/glext/glext.h"

#endif
