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

#include <glatter/glatter_config.h>
#include <glatter/glatter_system_headers.h>

#include <stdio.h>
#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif


////////////////////////////////////////////////////
#if defined(GLATTER_GL) || defined(GLATTER_GLES)  //
////////////////////////////////////////////////////

const char* enum_to_string_GL(GLenum e);

void glatter_check_error_GL(const char* file, int line)
{
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        printf("GLATTER: OpenGL call produced %s error in %s(%d)\n",
            enum_to_string_GL(err), file, line);
    }
}

void* glatter_get_proc_address_GL(const char* function_name)
{
    void* ptr = 0;
#if defined(GLATTER_EGL)
    ptr = (void*) eglGetProcAddress(function_name);
    if (ptr == 0) {
#if defined(_WIN32)
        ptr = (void*) GetProcAddress(GetModuleHandle(TEXT("libEGL.dll")), function_name);
#elif defined (__linux__)
        ptr = (void*) dlsym(dlopen("libEGL.so", RTLD_LAZY), function_name);
#else
#error There is no implementation for your platform. Your contribution would be greatly appreciated!
#endif
    }
#elif defined(GLATTER_WGL)
    ptr = (void*) wglGetProcAddress(function_name);
    if (ptr == 0)
        ptr = (void*) GetProcAddress(GetModuleHandle(TEXT("opengl32.dll")), function_name);
#elif defined(GLATTER_GLX)
    ptr = (void*) glXGetProcAddress(function_name);
    if (ptr == 0)
        ptr = (void*) dlsym(dlopen("libGL.so", RTLD_LAZY), function_name);
#endif
    return ptr;
}

//////////////////////////////////////////////////////////
#endif // defined(GLATTER_GL) || defined(GLATTER_GLES)  //
//////////////////////////////////////////////////////////

////////////////////////////
#if defined(GLATTER_GLX)  //
////////////////////////////

const char* enum_to_string_GLX(GLenum e);

#if !defined(GLATTER_DO_NOT_INSTALL_X_ERROR_HANDLER)
int x_error_handler(Display *dsp, XErrorEvent *error)
{
    char error_string[128];
    XGetErrorText(dsp, error->error_code, error_string, 128);
    printf("X Error: %s\n", error_string);
}
#endif //!defined(GLATTER_DO_NOT_INSTALL_X_ERROR_HANDLER)


void* glatter_get_proc_address_GLX_init(const char* function_name);
void* (*glatter_get_proc_address_GLX)(const char*) = glatter_get_proc_address_GLX_init;


void* glatter_get_proc_address_GLX_init(const char* function_name)
{
#if !defined(GLATTER_DO_NOT_INSTALL_X_ERROR_HANDLER)
    XSetErrorHandler(x_error_handler);
#endif //!defined(GLATTER_DO_NOT_INSTALL_X_ERROR_HANDLER)
    glatter_get_proc_address_GLX = glatter_get_proc_address_GL;
    return glatter_get_proc_address_GL(function_name);
}

void glatter_check_error_GLX(const char* file, int line)
{
    // TODO: decide if implementing further wrappers to be able
    // to call XSync here, is within the scope of this library.
}

//////////////////////////////////
#endif // defined(GLATTER_GLX)  //
//////////////////////////////////


////////////////////////////
#if defined(GLATTER_WGL)  //
////////////////////////////

const char* enum_to_string_WGL(GLenum e);

void glatter_check_error_WGL(const char* file, int line)
{
    DWORD eid = GetLastError();
    if(eid == 0)
        return;

    LPSTR buffer = 0;
    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS, NULL,
        eid, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&buffer, 0, NULL);

    printf("GLATTER: WGL call produced the following error in %s(%d):\n%s\t", file, line, buffer);

    LocalFree(buffer);
}


void* glatter_get_proc_address_WGL(const char* function_name)
{
    return glatter_get_proc_address_GL(function_name);
}

//////////////////////////////////
#endif // defined(GLATTER_WGL)  //
//////////////////////////////////

////////////////////////////
#if defined(GLATTER_EGL)  //
////////////////////////////

const char* enum_to_string_EGL(GLenum e);


void glatter_check_error_EGL(const char* file, int line)
{
    EGLint err = eglGetError();
    if (err != EGL_SUCCESS) {
        printf("GLATTER: EGL call produced %s error in %s(%d)\n",
            enum_to_string_EGL(err), file, line);
    }
}


void* glatter_get_proc_address_EGL(const char* function_name)
{
    return glatter_get_proc_address_GL(function_name);
}

//////////////////////////////////
#endif // defined(GLATTER_EGL)  //
//////////////////////////////////

void glatter_pre_callback(const char* file, int line)
{
    static int initialized = 0;
#if defined(_WIN32)
    static __declspec(thread) DWORD thread_id;
    if (!initialized) {
        thread_id = GetCurrentThreadId();
        initialized = 1;
    }
    DWORD current_thread = GetCurrentThreadId();
#elif defined(__linux__)
    static __thread pthread_t thread_id;
    if (!initialized) {
        thread_id = pthread_self();
        initialized = 1;
    }
    pthread_t current_thread = pthread_self();
#endif
    if (current_thread != thread_id) {
        printf("GLATTER: Calling OpenGL from a different thread, in %s(%d)\n", file, line);
    }
}


typedef struct
{
    char data[3*16+2];
}
Printable;


Printable get_prs(size_t sz, void* obj)
{
    Printable ret;
    ret.data[3*16+1] = 0;
    
    if (sz > 16)
        sz = 16;

    for (size_t i=0; i<sz; i++) {
        snprintf(&ret.data[i*3], sizeof(ret)-3*i-1," %02x", (unsigned char) *(((char*)obj)+i) );
    }
    
    ret.data[0] = '[';
    ret.data[3*sz] = ']';

    return ret;
}


#ifndef GLATTER_LOG
#define printf
#endif


#define GLATTER_FBLOCK(return_or_not, family, cder, rtype, cconv, name, cargs, dargs)\
    cder rtype cconv name dargs;\
    typedef rtype (cconv *glatter_##name##_t) dargs;\
    extern glatter_##name##_t glatter_##name##_ptr;\
    extern rtype cconv glatter_##name##_init dargs;\
    rtype cconv glatter_##name##_init dargs\
    {\
        glatter_##name##_ptr = (glatter_##name##_t) glatter_get_proc_address_##family(#name);\
        return_or_not glatter_##name##_ptr cargs;\
    }\
    glatter_##name##_t glatter_##name##_ptr = glatter_##name##_init;


#define GLATTER_DBLOCK(file, line, name, printf_fmt, ...) \
    glatter_pre_callback(file, line);\
    printf("GLATTER: calling in '%s'(%d):\n", file, line);\
    printf("GLATTER: " #name #printf_fmt "%s", __VA_ARGS__);


#define GET_PRS(v)\
    (get_prs(sizeof(v), (void*)(&(v))).data)


#if defined(__llvm__) || defined (__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-value"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsizeof-array-argument"
#endif

#include "glatter.c_gen"

#if defined(__llvm__) || defined (__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif


#ifdef __cplusplus
}
#endif
