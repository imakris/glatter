/*
Copyright 2017 Ioannis Makris

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __GLATTER_HEADER__

#include "glatter_config.h"
#include "glatter_system_headers.h"

#ifdef __cplusplus

extern "C" {

#ifdef GLATTER_HEADER_ONLY
    #define GLATTER_INCLUDED_FROM_HEADER
    #include "glatter_def.h"
#else
    #define GLATTER_INLINE_OR_NOT
#endif

#else

#ifdef GLATTER_HEADER_ONLY
    #error GLATTER_HEADER_ONLY can only be used in C++
#else
    #define GLATTER_INLINE_OR_NOT
#endif

#endif //__cplusplus


GLATTER_INLINE_OR_NOT const char* enum_to_string_GL(GLenum e);
GLATTER_INLINE_OR_NOT const char* enum_to_string_GLX(GLenum e);
GLATTER_INLINE_OR_NOT const char* enum_to_string_WGL(GLenum e);
GLATTER_INLINE_OR_NOT const char* enum_to_string_EGL(GLenum e);


#define GLATTER_UBLOCK(rtype, cconv, name, dargs)\
    typedef rtype (cconv *glatter_##name##_t) dargs;\
    extern glatter_##name##_t glatter_##name;


#if defined(GLATTER_LOG_ERRORS) || defined(GLATTER_LOG_CALLS)

    #if defined(GLATTER_GL)
    #include "generated/glatter_GL_d.h"
    #endif

    #if defined(GLATTER_GLX)
    #include "generated/glatter_GLX_d.h"
    #endif

    #if defined(GLATTER_EGL)
    #include "generated/glatter_EGL_d.h"
    #endif

    #if defined(GLATTER_WGL)
    #include "generated/glatter_WGL_d.h"
    #endif

    #if defined(GLATTER_GLU)
    #include "generated/glatter_GLU_d.h"
    #endif

#else

    #if defined(GLATTER_GL)
    #include "generated/glatter_GL_r.h"
    #endif

    #if defined(GLATTER_GLX)
    #include "generated/glatter_GLX_r.h"
    #endif

    #if defined(GLATTER_EGL)
    #include "generated/glatter_EGL_r.h"
    #endif

    #if defined(GLATTER_WGL)
    #include "generated/glatter_WGL_r.h"
    #endif

    #if defined(GLATTER_GLU)
    #include "generated/glatter_GLU_r.h"
    #endif

#endif


#ifdef __cplusplus
}
#endif

#endif
