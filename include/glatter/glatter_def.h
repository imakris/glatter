//NEW 1

/*
Copyright 2018 Ioannis Makris

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


#include <inttypes.h>

#include <glatter/glatter_config.h>
#include <glatter/glatter_platform_headers.h>
#include <glatter/glatter_masprintf.h>

#include <assert.h>
#include <stdio.h>
#include <stdarg.h>

/*
 * In non header-only builds, include this header only from glatter.c so the
 * non-inline definitions remain unique.
 */


#ifndef GLATTER_EXTERN_C_BEGIN
    #ifdef __cplusplus
        #define GLATTER_EXTERN_C_BEGIN extern "C" {
        #define GLATTER_EXTERN_C_END }
    #else
        #define GLATTER_EXTERN_C_BEGIN
        #define GLATTER_EXTERN_C_END
    #endif
#endif

GLATTER_EXTERN_C_BEGIN


 #ifdef GLATTER_HEADER_ONLY
    #undef GLATTER_INLINE_OR_NOT
    #define GLATTER_INLINE_OR_NOT inline
 #else
    #undef GLATTER_INLINE_OR_NOT
    #define GLATTER_INLINE_OR_NOT
 #endif

#if defined(GLATTER_HEADER_ONLY) && defined(__cplusplus)
    #if __cplusplus < 201103L && !defined(_MSC_VER)
        #error "Header-only mode requires C++11 for thread-safe local statics."
    #endif
#endif



GLATTER_INLINE_OR_NOT
void glatter_default_log_handler(const char* str)
{
    fprintf(stderr, "%s", str);
}


GLATTER_INLINE_OR_NOT
void (**glatter_log_handler_ptr_ptr())(const char*)
{
    static void(*handler_ptr)(const char*) = glatter_default_log_handler;
    return &handler_ptr;
}


GLATTER_INLINE_OR_NOT
void (*glatter_log_handler())(const char*)
{
    return *(glatter_log_handler_ptr_ptr());
}


GLATTER_INLINE_OR_NOT
const char* glatter_log(const char* str)
{
    const char* message = str;
    if (message == NULL) {
        static const char fallback[] = "GLATTER: message formatting failed.\n";
        message = fallback;
    }
    (*(glatter_log_handler_ptr_ptr()))(message);
    return str;
}


GLATTER_INLINE_OR_NOT
void glatter_log_printf(const char* fmt, ...)
{
    char buffer[2048];
    va_list args;
    va_start(args, fmt);
    int written = vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    if (written < 0) {
        glatter_log(NULL);
    } else {
        glatter_log(buffer);
    }
}


GLATTER_INLINE_OR_NOT
void glatter_set_log_handler(void(*handler_ptr)(const char*))
{
    if (handler_ptr == NULL) {
        handler_ptr = glatter_default_log_handler;
    }
    *(glatter_log_handler_ptr_ptr()) = handler_ptr;
}


GLATTER_INLINE_OR_NOT
void* glatter_get_proc_address(const char* function_name)
{
    void* ptr = 0;
#if defined(GLATTER_EGL)
    ptr = (void*) eglGetProcAddress(function_name);
    if (ptr == 0) {
#if defined(_WIN32)
        ptr = (void*) GetProcAddress(GetModuleHandle(TEXT("libEGL.dll")), function_name);
    }
    if (ptr == 0) {
        ptr = (void*) GetProcAddress(GetModuleHandle(TEXT("libGLES_CM.dll")), function_name);
#elif defined (__linux__)
        static void* egl_handle = NULL;
        static int egl_tried = 0;
        if (ptr == 0) {
            if (!egl_tried) {
                static const char* egl_sonames[] = {
                    "libEGL.so",
                    "libEGL.so.1",
                    NULL
                };
                for (int i = 0; egl_sonames[i] != NULL && egl_handle == NULL; ++i) {
                    egl_handle = dlopen(egl_sonames[i], RTLD_LAZY);
                }
                egl_tried = 1;
            }
            if (egl_handle != NULL) {
                ptr = (void*) dlsym(egl_handle, function_name);
            }
        }
    }
    if (ptr == 0) {
        static void* gles_handle = NULL;
        static int gles_tried = 0;
        if (!gles_tried) {
            static const char* gles_sonames[] = {
                "libGLES_CM.so",
                "libGLESv1_CM.so",
                "libGLESv1_CM.so.1",
                "libGLESv2.so",
                "libGLESv2.so.2",
                "libGLESv3.so",
                NULL
            };
            for (int i = 0; gles_sonames[i] != NULL && gles_handle == NULL; ++i) {
                gles_handle = dlopen(gles_sonames[i], RTLD_LAZY);
            }
            gles_tried = 1;
        }
        if (gles_handle != NULL) {
            ptr = (void*) dlsym(gles_handle, function_name);
        }
#else
#error There is no implementation for your platform. Your contribution would be greatly appreciated!
#endif
    }
#elif defined(GLATTER_WGL)
    ptr = (void*) wglGetProcAddress(function_name);
    if (ptr == 0)
        ptr = (void*) GetProcAddress(GetModuleHandle(TEXT("opengl32.dll")), function_name);
#elif defined(GLATTER_GLX)
    ptr = (void*) glXGetProcAddress((const GLubyte*)function_name);
    if (ptr == 0) {
        static void* gl_handle = NULL;
        static int gl_tried = 0;
        if (!gl_tried) {
            static const char* sons[] = { "libGL.so.1", "libGL.so", NULL };
            for (int i = 0; !gl_handle && sons[i]; ++i) gl_handle = dlopen(sons[i], RTLD_LAZY);
            gl_tried = 1;
        }
        if (gl_handle) ptr = (void*) dlsym(gl_handle, function_name);
    }
#endif
    return ptr;
}



///////////////////////////
#if defined(GLATTER_GL)  //
///////////////////////////

GLATTER_INLINE_OR_NOT
const char* enum_to_string_GL(GLATTER_ENUM_GL e);

GLATTER_INLINE_OR_NOT
void glatter_check_error_GL(const char* file, int line)
{
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        //printf("GLATTER: in '%s'(%d):\n", file, line);

        glatter_log_printf(
            "GLATTER: in '%s'(%d):\n", file, line
        );



        //printf("GLATTER: OpenGL call produced %s error.\n", enum_to_string_GL(err));
        glatter_log_printf(
            "GLATTER: OpenGL call produced %s error.\n", enum_to_string_GL(err)
        );

    }
}


GLATTER_INLINE_OR_NOT
void* glatter_get_proc_address_GL(const char* function_name)
{
    return glatter_get_proc_address(function_name);
}


////////////////////////////////
#endif // defined(GLATTER_GL) //
////////////////////////////////



////////////////////////////
#if defined(GLATTER_GLX)  //
////////////////////////////

GLATTER_INLINE_OR_NOT
const char* enum_to_string_GLX(GLATTER_ENUM_GLX e);

#if !defined(GLATTER_DO_NOT_INSTALL_X_ERROR_HANDLER)
GLATTER_INLINE_OR_NOT
int x_error_handler(Display *dsp, XErrorEvent *error)
{
    char error_string[128];
    XGetErrorText(dsp, error->error_code, error_string, 128);
    //printf("X Error: %s\n", error_string);

    glatter_log_printf(
        "X Error: %s\n", error_string
    );

    return 0;
}
#endif //!defined(GLATTER_DO_NOT_INSTALL_X_ERROR_HANDLER)


GLATTER_INLINE_OR_NOT
void* glatter_get_proc_address_GLX_init(const char* function_name);


GLATTER_INLINE_OR_NOT
void* glatter_get_proc_address_GLX(const char* function_name)
{
#if !defined(GLATTER_DO_NOT_INSTALL_X_ERROR_HANDLER)
    static int initialized = 0;
    if (!initialized) {
        XSetErrorHandler(x_error_handler);
        initialized = 1;
    }
#endif //!defined(GLATTER_DO_NOT_INSTALL_X_ERROR_HANDLER)

    return glatter_get_proc_address(function_name);
}


GLATTER_INLINE_OR_NOT
void glatter_check_error_GLX(const char* file, int line)
{
#if defined(GLATTER_LOG_ERRORS)
    /* X11/GLX errors are asynchronous. Force a round-trip so our
       X error handler runs for any pending errors. */
    Display* dpy = glXGetCurrentDisplay();
    if (dpy) {
        XSync(dpy, False);
    }
#else
    (void)file; (void)line; /* unused */
#endif
}

//////////////////////////////////
#endif // defined(GLATTER_GLX)  //
//////////////////////////////////


////////////////////////////
#if defined(GLATTER_WGL)  //
////////////////////////////

GLATTER_INLINE_OR_NOT
const char* enum_to_string_WGL(GLATTER_ENUM_WGL e);

GLATTER_INLINE_OR_NOT
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
        eid, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR*)&buffer, 0, NULL);

    //printf("GLATTER: WGL call produced the following error in %s(%d):\n%s\t", file, line, buffer);

    glatter_log_printf(
        "GLATTER: WGL call produced the following error in %s(%d):\n%s\t", file, line, buffer
    );

    LocalFree(buffer);
}


GLATTER_INLINE_OR_NOT
void* glatter_get_proc_address_WGL(const char* function_name)
{
    return glatter_get_proc_address(function_name);
}

//////////////////////////////////
#endif // defined(GLATTER_WGL)  //
//////////////////////////////////


////////////////////////////
#if defined(GLATTER_EGL)  //
////////////////////////////

GLATTER_INLINE_OR_NOT
const char* enum_to_string_EGL(GLATTER_ENUM_EGL e);


GLATTER_INLINE_OR_NOT
void glatter_check_error_EGL(const char* file, int line)
{
    EGLint err = eglGetError();
    if (err != EGL_SUCCESS) {
        //printf("GLATTER: EGL call produced %s error in %s(%d)\n",
        //    enum_to_string_EGL(err), file, line);

        glatter_log_printf(
            "GLATTER: EGL call produced %s error in %s(%d)\n", enum_to_string_EGL(err), file, line
        );
    }
}


GLATTER_INLINE_OR_NOT
void* glatter_get_proc_address_EGL(const char* function_name)
{
    return glatter_get_proc_address(function_name);
}

//////////////////////////////////
#endif // defined(GLATTER_EGL)  //
//////////////////////////////////

////////////////////////////
#if defined(GLATTER_GLU)  //
////////////////////////////

GLATTER_INLINE_OR_NOT
const char* enum_to_string_GLU(GLATTER_ENUM_GLU e);


GLATTER_INLINE_OR_NOT
void glatter_check_error_GLU(const char* file, int line)
{
    glatter_check_error_GL(file, line);
}


#if defined(__linux__)
static pthread_once_t* glatter_glu_once_slot(void)
{
    static pthread_once_t once = PTHREAD_ONCE_INIT;
    return &once;
}

static void** glatter_glu_handle_slot(void)
{
    static void* handle = NULL;
    return &handle;
}

/*
 * The GLU handle intentionally stays open for the lifetime of the process to
 * avoid any dlclose side effects with driver-managed resources.
 */
static void glatter_open_glu_handle(void)
{
    void** handle = glatter_glu_handle_slot();
    *handle = dlopen("libGLU.so", RTLD_LAZY | RTLD_LOCAL);
    if (*handle == NULL) {
        *handle = dlopen("libGLU.so.1", RTLD_LAZY | RTLD_LOCAL);
    }
}
#endif

GLATTER_INLINE_OR_NOT
void* glatter_get_proc_address_GLU(const char* function_name)
{
    void* ptr = NULL;

#if defined(__linux__)
    pthread_once(glatter_glu_once_slot(), glatter_open_glu_handle);
    void* handle = *glatter_glu_handle_slot();
    ptr = handle ? dlsym(handle, function_name) : NULL;
#elif defined(_WIN32)
    HMODULE module = GetModuleHandleA("glu32.dll");
    if (module == NULL) {
        module = LoadLibraryA("glu32.dll");
    }
    ptr = module ? (void*)GetProcAddress(module, function_name) : NULL;
#else
    #error There is no implementation for your platform. Your contribution would be greatly appreciated!
#endif

    return ptr;
}

//////////////////////////////////
#endif // defined(GLATTER_GLU)  //
//////////////////////////////////


// --- begin owner/thread block ---

GLATTER_EXTERN_C_END

/*
 * For header-only (C++) consumers we keep the thread ownership state inside a
 * function-local static so every translation unit observes the same cached
 * owner without needing an explicit instantiation macro.
 */
#if defined(GLATTER_HEADER_ONLY) && defined(__cplusplus)

namespace glatter { namespace detail {

#if defined(_WIN32)
    using thread_id_t = DWORD;

    inline thread_id_t current_thread_id()
    {
        return GetCurrentThreadId();
    }

    inline bool thread_ids_equal(thread_id_t a, thread_id_t b)
    {
        return a == b;
    }
#elif defined(__APPLE__) || defined(__unix__) || defined(__unix)
    using thread_id_t = pthread_t;

    inline thread_id_t current_thread_id()
    {
        return pthread_self();
    }

    inline bool thread_ids_equal(thread_id_t a, thread_id_t b)
    {
        return pthread_equal(a, b) != 0;
    }
#else
    #error "Unsupported platform"
#endif

    inline thread_id_t& owner_thread_id()
    {
        static thread_id_t tid = current_thread_id();
        return tid;
    }

    inline bool is_owner_thread()
    {
        return thread_ids_equal(current_thread_id(), owner_thread_id());
    }

    inline void bind_owner_to_current_thread()
    {
        owner_thread_id() = current_thread_id();
    }

}} // namespace glatter::detail

#else  // !header-only C++

GLATTER_EXTERN_C_BEGIN

#if defined(_WIN32)
    extern INIT_ONCE glatter_thread_once;
    extern DWORD     glatter_thread_id;

    static BOOL CALLBACK glatter_set_owner_thread(PINIT_ONCE once, PVOID param, PVOID* context)
    {
        (void)once;
        (void)param;
        (void)context;
        glatter_thread_id = GetCurrentThreadId();
        return TRUE;
    }
#elif defined(__APPLE__) || defined(__unix__) || defined(__unix)
    extern pthread_once_t glatter_thread_once;
    extern pthread_t      glatter_thread_id;

    static void glatter_set_owner_thread(void)
    {
        glatter_thread_id = pthread_self();
    }
#else
    #error "Unsupported platform"
#endif

GLATTER_EXTERN_C_END

#endif // header-only switch

// Re-enter C linkage for the remainder of the C API.
GLATTER_EXTERN_C_BEGIN

GLATTER_INLINE_OR_NOT
void glatter_pre_callback(const char* file, int line)
{
#if defined(GLATTER_HEADER_ONLY) && defined(__cplusplus)
    if (!glatter::detail::is_owner_thread()) {
        char* m = glatter_masprintf("GLATTER: Calling OpenGL from a different thread, in %s(%d)\n", file, line);
        glatter_log(m);
        free(m);
    }

#elif defined(_WIN32)
    if (!InitOnceExecuteOnce(&glatter_thread_once, glatter_set_owner_thread, NULL, NULL)) {
        return;
    }
    DWORD current_thread = GetCurrentThreadId();
    if (current_thread != glatter_thread_id) {
        char* m = glatter_masprintf("GLATTER: Calling OpenGL from a different thread, in %s(%d)\n", file, line);
        glatter_log(m);
        free(m);
    }

#elif defined(__APPLE__) || defined(__unix__) || defined(__unix)
    pthread_once(&glatter_thread_once, glatter_set_owner_thread);
    pthread_t current_thread = pthread_self();
    if (!pthread_equal(current_thread, glatter_thread_id)) {
        char* m = glatter_masprintf("GLATTER: Calling OpenGL from a different thread, in %s(%d)\n", file, line);
        glatter_log(m);
        free(m);
    }
#else
    #error "Unsupported platform"
#endif
}


GLATTER_INLINE_OR_NOT
void glatter_bind_owner_to_current_thread(void)
{
#if defined(GLATTER_HEADER_ONLY) && defined(__cplusplus)
    glatter::detail::bind_owner_to_current_thread();
#elif defined(_WIN32)
    glatter_thread_id = GetCurrentThreadId();
#elif defined(__APPLE__) || defined(__unix__) || defined(__unix)
    glatter_thread_id = pthread_self();
#endif
}

// --- end owner/thread block ---


typedef struct
{
    char data[3*16+2];
}
Printable;


GLATTER_INLINE_OR_NOT
Printable get_prs(size_t sz, void* obj)
{
    Printable ret;
    
    if (sz > 16)
        sz = 16;

    for (size_t i=0; i<sz; i++) {
        snprintf(&ret.data[i*3], sizeof(ret)-3*i-1," %02x", (unsigned char) *(((char*)obj)+i) );
    }
    
    ret.data[0     ] = '[';
    ret.data[3*sz  ] = ']';
    ret.data[3*sz+1] =  0 ;

    return ret;
}



GLATTER_INLINE_OR_NOT
uint32_t glatter_djb2(const uint8_t *str)
{
    uint32_t hash = 5381;

    for (int c = *str; c; c = *++str)
        hash = ((hash << 5) + hash) + c;

    return hash;
}


typedef struct glatter_es_record_struct
{
    uint32_t hash;
    uint16_t index;
} glatter_es_record_t;


//==================

#ifdef GLATTER_HEADER_ONLY


#define GLATTER_FBLOCK(return_or_not, family, cder, rtype, cconv, name, cargs, dargs)\
    typedef rtype (cconv *glatter_##name##_t) dargs;\
    inline rtype cconv glatter_##name dargs\
    {\
        static glatter_##name##_t s_f_ptr =\
            (glatter_##name##_t) glatter_get_proc_address_##family(#name);\
        return_or_not s_f_ptr cargs;\
    }


#else


#define GLATTER_FBLOCK(return_or_not, family, cder, rtype, cconv, name, cargs, dargs)\
    cder rtype cconv name dargs;\
    typedef rtype (cconv *glatter_##name##_t) dargs;\
    extern glatter_##name##_t glatter_##name ;\
    extern rtype cconv glatter_##name##_init dargs;\
    rtype cconv glatter_##name##_init dargs\
    {\
        glatter_##name = (glatter_##name##_t) glatter_get_proc_address_##family(#name);\
        return_or_not glatter_##name cargs;\
    }\
    glatter_##name##_t glatter_##name = glatter_##name##_init;


#endif

//==================

#if defined(GLATTER_LOG_CALLS)

    #define GLATTER_DBLOCK(file, line, name, printf_fmt, ...) \
        glatter_pre_callback(file, line);                     \
        glatter_log_printf(                                   \
            "GLATTER: in '%s'(%d):\n", file, line             \
        );                                                    \
        glatter_log_printf(                                   \
            "GLATTER: " #name printf_fmt "\n", ##__VA_ARGS__  \
        );

    #define GLATTER_RBLOCK(...)                               \
        glatter_log_printf(                                   \
            "GLATTER: returned " __VA_ARGS__                  \
        );


#else

    #define GLATTER_DBLOCK(file, line, name, printf_fmt, ...) \
        glatter_pre_callback(file, line);

    #define GLATTER_RBLOCK(...)

#endif


#if defined (GLATTER_LOG_ERRORS)

    #define GLATTER_CHECK_ERROR(family, file, line) glatter_check_error_##family(file, line);

#else

    #define GLATTER_CHECK_ERROR(...)

#endif


#define GET_PRS(v)\
    (get_prs(sizeof(v), (void*)(&(v))).data)


#if defined(__llvm__) || defined (__clang__)
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wunused-value"
#elif defined(__GNUC__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wsizeof-array-argument"
#endif


#ifndef GLATTER_HEADER_ONLY
    #define GLATTER_str(s) #s
    #define GLATTER_xstr(s) GLATTER_str(s)
    #define GLATTER_PDIR(pd) platforms/pd
#endif


#if defined(GLATTER_LOG_ERRORS) || defined(GLATTER_LOG_CALLS)

    #if defined(GLATTER_GL)
        #include GLATTER_xstr(GLATTER_PDIR(GLATTER_PLATFORM_DIR)/glatter_GL_d_def.h)
    #endif

    #if defined(GLATTER_GLX)
        #include GLATTER_xstr(GLATTER_PDIR(GLATTER_PLATFORM_DIR)/glatter_GLX_d_def.h)
    #endif

    #if defined(GLATTER_EGL)
        #include GLATTER_xstr(GLATTER_PDIR(GLATTER_PLATFORM_DIR)/glatter_EGL_d_def.h)
    #endif

    #if defined(GLATTER_WGL)
        #include GLATTER_xstr(GLATTER_PDIR(GLATTER_PLATFORM_DIR)/glatter_WGL_d_def.h)
    #endif

    #if defined(GLATTER_GLU)
        #include GLATTER_xstr(GLATTER_PDIR(GLATTER_PLATFORM_DIR)/glatter_GLU_d_def.h)
    #endif

#else

    #if defined(GLATTER_GL)
        #include GLATTER_xstr(GLATTER_PDIR(GLATTER_PLATFORM_DIR)/glatter_GL_r_def.h)
    #endif

    #if defined(GLATTER_GLX)
        #include GLATTER_xstr(GLATTER_PDIR(GLATTER_PLATFORM_DIR)/glatter_GLX_r_def.h)
    #endif

    #if defined(GLATTER_EGL)
        #include GLATTER_xstr(GLATTER_PDIR(GLATTER_PLATFORM_DIR)/glatter_EGL_r_def.h)
    #endif

    #if defined(GLATTER_WGL)
        #include GLATTER_xstr(GLATTER_PDIR(GLATTER_PLATFORM_DIR)/glatter_WGL_r_def.h)
    #endif

    #if defined(GLATTER_GLU)
        #include GLATTER_xstr(GLATTER_PDIR(GLATTER_PLATFORM_DIR)/glatter_GLU_r_def.h)
    #endif

#endif



#ifndef GLATTER_HEADER_ONLY

#if defined(GLATTER_GL)
    #include GLATTER_xstr(GLATTER_PDIR(GLATTER_PLATFORM_DIR)/glatter_GL_ges_decl.h)
#endif

#if defined(GLATTER_GLX)
    #include GLATTER_xstr(GLATTER_PDIR(GLATTER_PLATFORM_DIR)/glatter_GLX_ges_decl.h)
#endif

#if defined(GLATTER_EGL)
    #include GLATTER_xstr(GLATTER_PDIR(GLATTER_PLATFORM_DIR)/glatter_EGL_ges_decl.h)
#endif

#if defined(GLATTER_WGL)
    #include GLATTER_xstr(GLATTER_PDIR(GLATTER_PLATFORM_DIR)/glatter_WGL_ges_decl.h)
#endif

#endif



#if defined(GLATTER_GL)
    #include GLATTER_xstr(GLATTER_PDIR(GLATTER_PLATFORM_DIR)/glatter_GL_ges_def.h)
#endif

#if defined(GLATTER_GLX)
    #include GLATTER_xstr(GLATTER_PDIR(GLATTER_PLATFORM_DIR)/glatter_GLX_ges_def.h)
#endif

#if defined(GLATTER_EGL)
    #include GLATTER_xstr(GLATTER_PDIR(GLATTER_PLATFORM_DIR)/glatter_EGL_ges_def.h)
#endif

#if defined(GLATTER_WGL)
    #include GLATTER_xstr(GLATTER_PDIR(GLATTER_PLATFORM_DIR)/glatter_WGL_ges_def.h)
#endif



#if defined(GLATTER_GL)
    #include GLATTER_xstr(GLATTER_PDIR(GLATTER_PLATFORM_DIR)/glatter_GL_e2s_def.h)
#endif

#if defined(GLATTER_GLX)
    #include GLATTER_xstr(GLATTER_PDIR(GLATTER_PLATFORM_DIR)/glatter_GLX_e2s_def.h)
#endif

#if defined(GLATTER_EGL)
    #include GLATTER_xstr(GLATTER_PDIR(GLATTER_PLATFORM_DIR)/glatter_EGL_e2s_def.h)
#endif

#if defined(GLATTER_WGL)
    #include GLATTER_xstr(GLATTER_PDIR(GLATTER_PLATFORM_DIR)/glatter_WGL_e2s_def.h)
#endif

#if defined(GLATTER_GLU)
    #include GLATTER_xstr(GLATTER_PDIR(GLATTER_PLATFORM_DIR)/glatter_GLU_e2s_def.h)
#endif


#if defined(__llvm__) || defined (__clang__)
    #pragma clang diagnostic pop
#elif defined(__GNUC__)
    #pragma GCC diagnostic pop
#endif


GLATTER_EXTERN_C_END
