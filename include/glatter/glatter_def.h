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
#include <glatter/glatter_atomic.h>
#include <glatter/glatter_platform_headers.h>
#include <glatter/glatter_masprintf.h>

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#if defined(_WIN32)
#   include <windows.h>
#   include <tchar.h>
#   if !defined(LOAD_LIBRARY_SEARCH_SYSTEM32)
#       define LOAD_LIBRARY_SEARCH_SYSTEM32 0x00000800
#   endif
#   if !defined(GLATTER_THREAD_LOCAL)
#       if defined(_MSC_VER)
#           define GLATTER_THREAD_LOCAL __declspec(thread)
#       elif defined(__GNUC__)
#           define GLATTER_THREAD_LOCAL __thread
#       else
#           define GLATTER_THREAD_LOCAL
#           if !defined(GLATTER_NO_TLS_WARNING)
#               define GLATTER_NO_TLS_WARNING
#               if defined(_MSC_VER)
#                   pragma message("glatter: TLS not detected for this compiler; some logs may interleave across threads.")
#               else
#                   warning "glatter: TLS not detected for this compiler; some logs may interleave across threads."
#               endif
#           endif
#       endif
#   endif
#else
#   if !defined(GLATTER_THREAD_LOCAL)
#       if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
#           define GLATTER_THREAD_LOCAL _Thread_local
#       elif defined(__GNUC__)
#           define GLATTER_THREAD_LOCAL __thread
#       else
#           define GLATTER_THREAD_LOCAL
#           if !defined(GLATTER_NO_TLS_WARNING)
#               define GLATTER_NO_TLS_WARNING
#               if defined(_MSC_VER)
#                   pragma message("glatter: TLS not detected for this compiler; some logs may interleave across threads.")
#               else
#                   warning "glatter: TLS not detected for this compiler; some logs may interleave across threads."
#               endif
#           endif
#       endif
#   endif
#endif

#undef GLATTER_HAS_ATOMIC_LOG_HANDLER
#if defined(__cplusplus)
#   if __cplusplus >= 201103L
#       define GLATTER_HAS_ATOMIC_LOG_HANDLER 1
#       if !defined(GLATTER_HEADER_ONLY)
#           include <atomic>
#       endif
#   else
#       define GLATTER_HAS_ATOMIC_LOG_HANDLER 0
#   endif
#else
#   if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L && !defined(__STDC_NO_ATOMICS__)
#       define GLATTER_HAS_ATOMIC_LOG_HANDLER 1
#       include <stdatomic.h>
#   else
#       define GLATTER_HAS_ATOMIC_LOG_HANDLER 0
#   endif
#endif

/* LOG HANDLER POLICY (no atomics):
 * If atomics are unavailable, glatter_set_log_handler() must be called during
 * single-threaded startup. After that, do not mutate it from multiple threads.
 * This configuration is supported and guarded by this build-time warning.
 */
#if !GLATTER_HAS_ATOMIC_LOG_HANDLER && !defined(GLATTER_LOG_HANDLER_INIT_WARNING)
#   define GLATTER_LOG_HANDLER_INIT_WARNING 1
#   if defined(_MSC_VER)
#       pragma message("GLATTER: Atomics unavailable; call glatter_set_log_handler() during single-threaded initialization.")
#   else
#       warning "GLATTER: Atomics unavailable; call glatter_set_log_handler() during single-threaded initialization."
#   endif
#endif

#if defined(__linux__) || defined(__unix__) || defined(__APPLE__)
    #include <dlfcn.h>
    #include <pthread.h>
#endif

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
    #define GLATTER_INLINE_OR_NOT static inline
 #else
    #undef GLATTER_INLINE_OR_NOT
    #define GLATTER_INLINE_OR_NOT
 #endif

#if defined(GLATTER_HEADER_ONLY) && defined(__cplusplus)
    #if __cplusplus < 201103L && !defined(_MSC_VER)
        #error "Header-only mode requires C++11 for thread-safe local statics."
    #endif
    #if defined(_MSC_VER) && _MSC_VER < 1900
        #error "Header-only mode requires MSVC 2015 (19.0) or newer."
    #endif
#endif



GLATTER_INLINE_OR_NOT
void glatter_default_log_handler(const char* str)
{
    fprintf(stderr, "%s", str);
}


typedef void (*glatter_log_handler_fn)(const char*);

#if GLATTER_HAS_ATOMIC_LOG_HANDLER
#   if defined(__cplusplus)
        using glatter_log_handler_atomic_t = std::atomic<glatter_log_handler_fn>;

GLATTER_INLINE_OR_NOT
glatter_log_handler_atomic_t& glatter_log_handler_storage(void)
{
    static glatter_log_handler_atomic_t handler(glatter_default_log_handler);
    return handler;
}

GLATTER_INLINE_OR_NOT
void glatter_log_handler_store(glatter_log_handler_fn handler_ptr)
{
    glatter_log_handler_storage().store(handler_ptr, std::memory_order_release);
}

GLATTER_INLINE_OR_NOT
glatter_log_handler_fn glatter_log_handler_load(void)
{
    return glatter_log_handler_storage().load(std::memory_order_acquire);
}
#   else
        typedef _Atomic(glatter_log_handler_fn) glatter_log_handler_atomic_t;

GLATTER_INLINE_OR_NOT
glatter_log_handler_atomic_t* glatter_log_handler_storage(void)
{
    static glatter_log_handler_atomic_t handler = glatter_default_log_handler;
    return &handler;
}

GLATTER_INLINE_OR_NOT
void glatter_log_handler_store(glatter_log_handler_fn handler_ptr)
{
    atomic_store_explicit(glatter_log_handler_storage(), handler_ptr, memory_order_release);
}

GLATTER_INLINE_OR_NOT
glatter_log_handler_fn glatter_log_handler_load(void)
{
    return atomic_load_explicit(glatter_log_handler_storage(), memory_order_acquire);
}
#   endif
#else

GLATTER_INLINE_OR_NOT
glatter_log_handler_fn* glatter_log_handler_storage(void)
{
    static glatter_log_handler_fn handler = glatter_default_log_handler;
    return &handler;
}

static int glatter_log_handler_frozen = 0;

GLATTER_INLINE_OR_NOT
void glatter_log_handler_store(glatter_log_handler_fn handler_ptr)
{
    *glatter_log_handler_storage() = handler_ptr;
}

GLATTER_INLINE_OR_NOT
glatter_log_handler_fn glatter_log_handler_load(void)
{
    glatter_log_handler_frozen = 1;
    return *glatter_log_handler_storage();
}
#endif


GLATTER_INLINE_OR_NOT
void (*glatter_log_handler())(const char*)
{
    return glatter_log_handler_load();
}


GLATTER_INLINE_OR_NOT
const char* glatter_log(const char* str)
{
    const char* message = str;
    if (message == NULL) {
        static const char fallback[] = "GLATTER: message formatting failed.\n";
        message = fallback;
    }
    glatter_log_handler_fn handler = glatter_log_handler_load();
    handler(message);
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
    }
    else {
        glatter_log(buffer);
    }
}


GLATTER_INLINE_OR_NOT
void glatter_set_log_handler(void(*handler_ptr)(const char*))
{
#if !GLATTER_HAS_ATOMIC_LOG_HANDLER
    /* No-atomics build: handler may only be set before first log; later changes are ignored. */
    if (glatter_log_handler_frozen) {
#if defined(GLATTER_DEBUG)
        glatter_log("GLATTER: ignoring log handler change after first use (no atomics).\n");
#endif
        return;
    }
#endif
    if (handler_ptr == NULL) {
        handler_ptr = glatter_default_log_handler;
    }
    glatter_log_handler_store(handler_ptr);
}

#if defined(_WIN32)
/* -------- TCHAR* logging helper (Windows only) -------- */
/* Prints TCHAR* as UTF-8 for logging.
 * Uses a small TLS ring so multiple %s in one call do not alias.
 */
static const char* glatter_pr_tstr(const TCHAR* ts)
{
    if (!ts) {
        return "(null)";
    }
#if defined(UNICODE) || defined(_UNICODE)
    enum { GLATTER_TSTR_SLOTS = 8, GLATTER_TSTR_CAP = 1024 };
    static GLATTER_THREAD_LOCAL char  tbuf[GLATTER_TSTR_SLOTS][GLATTER_TSTR_CAP];
    static GLATTER_THREAD_LOCAL unsigned tix;
    char* buf = tbuf[tix++ % GLATTER_TSTR_SLOTS];
    /* Convert UTF-16 -> UTF-8 */
    int n = WideCharToMultiByte(CP_UTF8, 0, ts, -1, buf, (int)GLATTER_TSTR_CAP, NULL, NULL);
    if (n > 0) {
        return buf;
    }
    /* Fallback on conversion failure */
    buf[0] = '?';
    buf[1] = '\0';
    return buf;
#else
    return (const char*)ts;
#endif
}
#endif /* _WIN32 */


typedef int glatter_wsi_t;

#ifndef GLATTER_WSI_AUTO_VALUE
#define GLATTER_WSI_AUTO_VALUE 0
#define GLATTER_WSI_WGL_VALUE  1
#define GLATTER_WSI_GLX_VALUE  2
#define GLATTER_WSI_EGL_VALUE  3
#endif

#if defined(_WIN32)
typedef void* (WINAPI *glatter_egl_get_proc_fn)(const char*);
static const char* const glatter_windows_egl_names[] = {
    "libEGL.dll",
    "EGL.dll"
};

#define GLATTER_WINDOWS_GLES_MODULE_COUNT 3
static const char* const glatter_windows_gles_names[GLATTER_WINDOWS_GLES_MODULE_COUNT][4] = {
    { "libGLESv3.dll", "GLESv3.dll", NULL, NULL },
    { "libGLESv2.dll", "GLESv2.dll", NULL, NULL },
    { "libGLESv1_CM.dll", "GLESv1_CM.dll", "libGLES_CM.dll", "GLES_CM.dll" }
};
#else
#define GLATTER_GL_SONAME_COUNT 2
#define GLATTER_EGL_SONAME_COUNT 2
#define GLATTER_GLES_SONAME_COUNT 8
static const char* const glatter_gl_sonames[GLATTER_GL_SONAME_COUNT] = {
    "libGL.so.1",
    "libGL.so"
};
static const char* const glatter_egl_sonames[GLATTER_EGL_SONAME_COUNT] = {
    "libEGL.so.1",
    "libEGL.so"
};
static const char* const glatter_gles_sonames[GLATTER_GLES_SONAME_COUNT] = {
    "libGLESv3.so.3",
    "libGLESv3.so",
    "libGLESv2.so.2",
    "libGLESv2.so",
    "libGLESv1_CM.so.1",
    "libGLESv1_CM.so",
    "libGLES_CM.so.1",
    "libGLES_CM.so"
};
#endif

typedef struct glatter_loader_state {
    glatter_wsi_t requested;
    glatter_wsi_t active;
    int wsi_explicit;
    int env_checked;
#if defined(_WIN32)
    HMODULE opengl32_module;
    HMODULE egl_module;
    HMODULE gles_modules[GLATTER_WINDOWS_GLES_MODULE_COUNT];
    PROC (WINAPI *wgl_get_proc)(LPCSTR);
    glatter_egl_get_proc_fn egl_get_proc;
#else
    /* Loader handles remain open for the process lifetime by design. We do not
     * dlclose/FreeLibrary to avoid driver/layer teardown side effects.
     */
    void* gl_handles[GLATTER_GL_SONAME_COUNT];
    unsigned char gl_tried[GLATTER_GL_SONAME_COUNT];
    void* egl_handles[GLATTER_EGL_SONAME_COUNT];
    unsigned char egl_tried[GLATTER_EGL_SONAME_COUNT];
    void* gles_handles[GLATTER_GLES_SONAME_COUNT];
    unsigned char gles_tried[GLATTER_GLES_SONAME_COUNT];
    void* (*glx_get_proc)(const GLubyte*);
    void* (*egl_get_proc)(const char*);
#endif
#if defined(GLATTER_GLX) && !defined(GLATTER_DO_NOT_INSTALL_X_ERROR_HANDLER)
    int glx_error_handler_installed;
#endif
} glatter_loader_state;

/* THREADING NOTE:
 * Per-entry resolution uses atomic CAS and is thread-safe.
 * Loader state is static and updated in a read-mostly, idempotent way.
 */
static glatter_loader_state* glatter_loader_state_get(void)
{
    static glatter_loader_state state = {
#if defined(GLATTER_WGL) && !defined(GLATTER_GLX) && !defined(GLATTER_EGL)
        GLATTER_WSI_WGL_VALUE,
#elif defined(GLATTER_GLX) && !defined(GLATTER_EGL) && !defined(GLATTER_WGL)
        GLATTER_WSI_GLX_VALUE,
#elif defined(GLATTER_EGL) && !defined(GLATTER_GLX) && !defined(GLATTER_WGL)
        GLATTER_WSI_EGL_VALUE,
#else
        GLATTER_WSI_AUTO_VALUE,
#endif
        GLATTER_WSI_AUTO_VALUE,
#if (defined(GLATTER_WGL) && !defined(GLATTER_GLX) && !defined(GLATTER_EGL)) || \
    (defined(GLATTER_GLX) && !defined(GLATTER_EGL) && !defined(GLATTER_WGL)) || \
    (defined(GLATTER_EGL) && !defined(GLATTER_GLX) && !defined(GLATTER_WGL))
        /* wsi_explicit: exactly one WSI is compiled-in -> ignore GLATTER_WSI */
        1,
#else
        /* wsi_explicit: multiple WSIs possible -> allow GLATTER_WSI to steer */
        0,
#endif
        /* env_checked */ 0
    };
    return &state;
}

static int glatter_equals_ignore_case(const char* a, const char* b)
{
    if (!a || !b) {
        return 0;
    }
    while (*a && *b) {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) {
            return 0;
        }
        ++a;
        ++b;
    }
    return *a == '\0' && *b == '\0';
}

/* Environment steering: we read GLATTER_WSI once per process unless the app
 * has already called glatter_set_wsi(), in which case environment input is
 * ignored. Explicit app choice takes precedence.
 */
static void glatter_detect_wsi_from_env(glatter_loader_state* state)
{
    if (state->env_checked || state->wsi_explicit) {
        state->env_checked = 1;
        return;
    }

    state->env_checked = 1;
    const char* env = getenv("GLATTER_WSI");
    if (!env || !*env) {
        return;
    }

    if (glatter_equals_ignore_case(env, "wgl")) {
        state->requested = GLATTER_WSI_WGL_VALUE;
    }
    else
    if (glatter_equals_ignore_case(env, "glx")) {
        state->requested = GLATTER_WSI_GLX_VALUE;
    }
    else
    if (glatter_equals_ignore_case(env, "egl")) {
        state->requested = GLATTER_WSI_EGL_VALUE;
    }

#if defined(_WIN32)
    /* Windows has no GLX; ignore such requests to avoid confusing state. */
    if (state->requested == GLATTER_WSI_GLX_VALUE) {
        state->requested = GLATTER_WSI_AUTO_VALUE;
    }
#endif

#if !defined(_WIN32)
    /* If this POSIX build was compiled without GLX, normalize invalid requests. */
#if !defined(GLATTER_GLX)
    if (state->requested == GLATTER_WSI_GLX_VALUE) {
        state->requested = GLATTER_WSI_AUTO_VALUE;
    }
#endif
#if !defined(GLATTER_EGL)
    if (state->requested == GLATTER_WSI_EGL_VALUE) {
        state->requested = GLATTER_WSI_AUTO_VALUE;
    }
#endif
#endif
}

#if defined(_WIN32)
/* Secure-by-default DLL load (Windows):
 * - Prefer LOAD_LIBRARY_SEARCH_SYSTEM32 when available.
 * - Fallback: build an absolute path to %SystemRoot%\System32.
 * - Define GLATTER_UNSAFE_DLL_SEARCH to restore legacy, unsafe search.
 */
static HMODULE glatter_load_system32_dll_(const wchar_t* dll_w)
{
#if !defined(GLATTER_UNSAFE_DLL_SEARCH)
    HMODULE h = LoadLibraryExW(dll_w, NULL, LOAD_LIBRARY_SEARCH_SYSTEM32);
    if (h) {
        return h;
    }

    wchar_t sysdir[MAX_PATH];
    UINT n = GetSystemDirectoryW(sysdir, MAX_PATH);
    if (n > 0 && n < MAX_PATH) {
        wchar_t full[MAX_PATH];
        if (swprintf(full, MAX_PATH, L"%s\\%s", sysdir, dll_w) > 0) {
            h = LoadLibraryW(full);
            if (h) {
                return h;
            }
        }
    }

    return LoadLibraryW(dll_w);
#else
    return LoadLibraryW(dll_w);
#endif
}

#ifdef GLATTER_SECURE_DLL_SEARCH
/* Deprecated: secure loading is now always on. */
#endif

static HMODULE glatter_windows_load_module(HMODULE* cache, const char* const* names, size_t count)
{
    if (*cache) {
        return *cache;
    }
    for (size_t i = 0; i < count; ++i) {
        const char* name = names[i];
        if (!name || !*name) {
            continue;
        }
        HMODULE module = GetModuleHandleA(name);
        if (!module) {
            wchar_t dll_w[MAX_PATH];
            size_t len = 0;
            for (; len < (size_t)(MAX_PATH - 1) && name[len]; ++len) {
                dll_w[len] = (wchar_t)(unsigned char)name[len];
            }
            dll_w[len] = L'\0';
            if (name[len] == '\0') {
                module = glatter_load_system32_dll_(dll_w);
            }
        }
        if (module) {
            *cache = module;
            return module;
        }
    }
    return NULL;
}

static void* glatter_windows_resolve_wgl(glatter_loader_state* state, const char* name)
{
    static const char* const opengl_names[] = { "opengl32.dll" };
    glatter_windows_load_module(&state->opengl32_module, opengl_names, sizeof(opengl_names) / sizeof(opengl_names[0]));

    if (state->opengl32_module && !state->wgl_get_proc) {
        state->wgl_get_proc = (PROC (WINAPI*)(LPCSTR))GetProcAddress(state->opengl32_module, "wglGetProcAddress");
    }

    if (state->wgl_get_proc) {
        PROC proc = state->wgl_get_proc(name);
        if (proc && proc != (PROC)1 && proc != (PROC)2 && proc != (PROC)3 && proc != (PROC)4 && proc != (PROC)-1) {
            return (void*)proc;
        }
    }

    if (state->opengl32_module) {
        PROC proc = (PROC)GetProcAddress(state->opengl32_module, name);
        if (proc) {
            return (void*)proc;
        }
    }

    return NULL;
}

static void* glatter_windows_resolve_egl(glatter_loader_state* state, const char* name)
{
    glatter_windows_load_module(&state->egl_module, glatter_windows_egl_names, sizeof(glatter_windows_egl_names) / sizeof(glatter_windows_egl_names[0]));

    if (state->egl_module && !state->egl_get_proc) {
        state->egl_get_proc = (glatter_egl_get_proc_fn)GetProcAddress(state->egl_module, "eglGetProcAddress");
    }

    if (state->egl_get_proc) {
        void* sym = state->egl_get_proc(name);
        if (sym) {
            return sym;
        }
    }

    if (state->egl_module) {
        void* sym = (void*)GetProcAddress(state->egl_module, name);
        if (sym) {
            return sym;
        }
    }

    if (name && strncmp(name, "gl", 2) == 0) {
        for (size_t i = 0; i < GLATTER_WINDOWS_GLES_MODULE_COUNT; ++i) {
            glatter_windows_load_module(&state->gles_modules[i], glatter_windows_gles_names[i], sizeof(glatter_windows_gles_names[i]) / sizeof(glatter_windows_gles_names[i][0]));
            if (state->gles_modules[i]) {
                void* sym = (void*)GetProcAddress(state->gles_modules[i], name);
                if (sym) {
                    return sym;
                }
            }
        }
    }

    return NULL;
}
#else
static void glatter_linux_load_handles(void** handles, unsigned char* tried, size_t count, const char* const* names)
{
    for (size_t i = 0; i < count; ++i) {
        if (!tried[i]) {
            handles[i] = dlopen(names[i], RTLD_LAZY | RTLD_LOCAL);
            tried[i] = 1;
        }
    }
}

static void* glatter_linux_lookup_in_handles(void** handles, size_t count, const char* name)
{
    for (size_t i = 0; i < count; ++i) {
        if (handles[i]) {
            void* sym = dlsym(handles[i], name);
            if (sym) {
                return sym;
            }
        }
    }
    return NULL;
}

static void* glatter_linux_lookup_glx(glatter_loader_state* state, const char* name)
{
    glatter_linux_load_handles(state->gl_handles, state->gl_tried, GLATTER_GL_SONAME_COUNT, glatter_gl_sonames);

    if (!state->glx_get_proc) {
        for (size_t i = 0; i < GLATTER_GL_SONAME_COUNT; ++i) {
            if (state->gl_handles[i]) {
                state->glx_get_proc = (void* (*)(const GLubyte*))dlsym(state->gl_handles[i], "glXGetProcAddressARB");
                if (!state->glx_get_proc) {
                    state->glx_get_proc = (void* (*)(const GLubyte*))dlsym(state->gl_handles[i], "glXGetProcAddress");
                }
                if (state->glx_get_proc) {
                    break;
                }
            }
        }
    }

    if (state->glx_get_proc) {
        void* sym = state->glx_get_proc((const GLubyte*)name);
        if (sym) {
            return sym;
        }
    }

    return glatter_linux_lookup_in_handles(state->gl_handles, GLATTER_GL_SONAME_COUNT, name);
}

static void* glatter_linux_lookup_egl(glatter_loader_state* state, const char* name)
{
    glatter_linux_load_handles(state->egl_handles, state->egl_tried, GLATTER_EGL_SONAME_COUNT, glatter_egl_sonames);

    if (!state->egl_get_proc) {
        for (size_t i = 0; i < GLATTER_EGL_SONAME_COUNT; ++i) {
            if (state->egl_handles[i]) {
                state->egl_get_proc = (void* (*)(const char*))dlsym(state->egl_handles[i], "eglGetProcAddress");
                if (state->egl_get_proc) {
                    break;
                }
            }
        }
    }

    if (state->egl_get_proc) {
        void* sym = state->egl_get_proc(name);
        if (sym) {
            return sym;
        }
    }

    void* sym = glatter_linux_lookup_in_handles(state->egl_handles, GLATTER_EGL_SONAME_COUNT, name);
    if (sym) {
        return sym;
    }

    if (name && strncmp(name, "gl", 2) == 0) {
        glatter_linux_load_handles(state->gles_handles, state->gles_tried, GLATTER_GLES_SONAME_COUNT, glatter_gles_sonames);
        return glatter_linux_lookup_in_handles(state->gles_handles, GLATTER_GLES_SONAME_COUNT, name);
    }

    return NULL;
}
#endif

GLATTER_INLINE_OR_NOT
void glatter_set_wsi(int wsi)
{
    glatter_loader_state* state = glatter_loader_state_get();
    glatter_wsi_t value = (glatter_wsi_t)wsi;
    switch (value) {
        case GLATTER_WSI_WGL_VALUE:
        case GLATTER_WSI_GLX_VALUE:
        case GLATTER_WSI_EGL_VALUE:
        case GLATTER_WSI_AUTO_VALUE:
            break;
        default:
            value = GLATTER_WSI_AUTO_VALUE;
            break;
    }
    state->requested = value;
    state->wsi_explicit = 1;
    state->active = GLATTER_WSI_AUTO_VALUE;
}

GLATTER_INLINE_OR_NOT
int glatter_get_wsi(void)
{
    glatter_loader_state* state = glatter_loader_state_get();
    glatter_detect_wsi_from_env(state);
    if (state->active != GLATTER_WSI_AUTO_VALUE) {
        return state->active;
    }
    return state->requested;
}

GLATTER_INLINE_OR_NOT
void* glatter_get_proc_address(const char* function_name)
{
    glatter_loader_state* state = glatter_loader_state_get();
    glatter_detect_wsi_from_env(state);

#if defined(_WIN32)
    if (state->requested == GLATTER_WSI_WGL_VALUE) {
        void* ptr = glatter_windows_resolve_wgl(state, function_name);
        if (ptr) {
            state->active = GLATTER_WSI_WGL_VALUE;
        }
        return ptr;
    }

    if (state->requested == GLATTER_WSI_EGL_VALUE) {
        void* ptr = glatter_windows_resolve_egl(state, function_name);
        if (ptr) {
            state->active = GLATTER_WSI_EGL_VALUE;
        }
        return ptr;
    }

    /* AUTO (Windows): try WGL first, then EGL. */
    void* ptr = glatter_windows_resolve_wgl(state, function_name);
    if (ptr) {
        state->active = GLATTER_WSI_WGL_VALUE;
        if (state->requested == GLATTER_WSI_AUTO_VALUE && !state->wsi_explicit) {
            /* Lock AUTO to the first successful WSI to avoid accidental WGL/EGL mixing. */
            state->requested = state->active;
            state->wsi_explicit = 1;
        }
        return ptr;
    }

    ptr = glatter_windows_resolve_egl(state, function_name);
    if (ptr) {
        state->active = GLATTER_WSI_EGL_VALUE;
        if (state->requested == GLATTER_WSI_AUTO_VALUE && !state->wsi_explicit) {
            /* Lock AUTO to the first successful WSI to avoid accidental WGL/EGL mixing. */
            state->requested = state->active;
            state->wsi_explicit = 1;
        }
        return ptr;
    }

    return NULL;
#else
    if (state->requested == GLATTER_WSI_GLX_VALUE) {
        void* ptr = glatter_linux_lookup_glx(state, function_name);
        if (ptr) {
            state->active = GLATTER_WSI_GLX_VALUE;
        }
        return ptr;
    }

    if (state->requested == GLATTER_WSI_EGL_VALUE) {
        void* ptr = glatter_linux_lookup_egl(state, function_name);
        if (ptr) {
            state->active = GLATTER_WSI_EGL_VALUE;
        }
        return ptr;
    }

    /* AUTO (POSIX): try GLX first, then EGL. */
    void* ptr = glatter_linux_lookup_glx(state, function_name);
    if (ptr) {
        state->active = GLATTER_WSI_GLX_VALUE;
        if (state->requested == GLATTER_WSI_AUTO_VALUE && !state->wsi_explicit) {
            /* Lock AUTO to the first successful WSI to avoid accidental WGL/EGL mixing. */
            state->requested = state->active;
            state->wsi_explicit = 1;
        }
        return ptr;
    }

    ptr = glatter_linux_lookup_egl(state, function_name);
    if (ptr) {
        state->active = GLATTER_WSI_EGL_VALUE;
        if (state->requested == GLATTER_WSI_AUTO_VALUE && !state->wsi_explicit) {
            /* Lock AUTO to the first successful WSI to avoid accidental WGL/EGL mixing. */
            state->requested = state->active;
            state->wsi_explicit = 1;
        }
        return ptr;
    }

    return NULL;
#endif
}



///////////////////////////
#if defined(GLATTER_GL)  //
///////////////////////////

GLATTER_INLINE_OR_NOT
const char* enum_to_string_GL(GLATTER_ENUM_GL e);

GLATTER_INLINE_OR_NOT
void glatter_check_error_GL(const char* file, int line)
{
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        glatter_log_printf(
            "GLATTER: in '%s'(%d):\n", file, line
        );

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
    glatter_log_printf(
        "X Error: %s\n", error_string
    );

    return 0;
}
#endif //!defined(GLATTER_DO_NOT_INSTALL_X_ERROR_HANDLER)


GLATTER_INLINE_OR_NOT
void* glatter_get_proc_address_GLX(const char* function_name)
{
    void* ptr = glatter_get_proc_address(function_name);

#if !defined(GLATTER_DO_NOT_INSTALL_X_ERROR_HANDLER)
    if (ptr) {
        glatter_loader_state* state = glatter_loader_state_get();
        if (!state->glx_error_handler_installed && state->active == GLATTER_WSI_GLX_VALUE) {
            /* GLX error handler is process-global; re-installing same function is benign.
             * We avoid pthread_once to keep header-only simple; opt out via GLATTER_DO_NOT_INSTALL_X_ERROR_HANDLER.
             */
            XSetErrorHandler(x_error_handler);
            glatter_log("GLATTER: installed default X error handler (define GLATTER_DO_NOT_INSTALL_X_ERROR_HANDLER to disable).\n");
            state->glx_error_handler_installed = 1;
        }
    }
#endif //!defined(GLATTER_DO_NOT_INSTALL_X_ERROR_HANDLER)

    return ptr;
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
    /* Diagnostics note:
       WGL entry points do not share a single failure contract. Here we only log
       GetLastError() as best-effort diagnostics. We deliberately do NOT attempt to
       interpret per-function return values (that would be brittle and often wrong).
       Callers should interpret the return value according to the specific API they used. */
    DWORD eid = GetLastError();
    if(eid == 0)
        return;

    LPVOID buffer = NULL;
    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS, NULL,
        eid, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&buffer, 0, NULL);

    glatter_log_printf(
        "GLATTER: WGL call produced the following error in %s(%d):\n%s\t", file, line, (char*)buffer
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
        module = glatter_load_system32_dll_(L"glu32.dll");
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

GLATTER_EXTERN_C_BEGIN
#if defined(_MSC_VER)
__declspec(selectany) extern const int glatter_build_mode_separate = 2;
#elif defined(__GNUC__)
extern const int glatter_build_mode_separate __attribute__((weak, visibility("default"))) = 2;
#else
extern const int glatter_build_mode_separate = 2;
#endif
/* Link-time guard: fails the build if both header-only and separate-build are linked. */
GLATTER_EXTERN_C_END

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

    /* Ownership policy:
     * - Default: first caller becomes owner (simple apps).
     * - Define GLATTER_REQUIRE_EXPLICIT_OWNER_BIND to force explicit binding,
     *   then call glatter_bind_owner_to_current_thread() on your render thread.
     */
    inline thread_id_t& owner_thread_id()
    {
#if defined(GLATTER_REQUIRE_EXPLICIT_OWNER_BIND)
        static thread_id_t tid = thread_id_t();
#else
        static thread_id_t tid = current_thread_id();
#endif
        return tid;
    }

#if defined(GLATTER_REQUIRE_EXPLICIT_OWNER_BIND)
    inline bool& owner_thread_bound()
    {
        static bool bound = false;
        return bound;
    }
#endif

    inline bool is_owner_thread()
    {
#if defined(GLATTER_REQUIRE_EXPLICIT_OWNER_BIND)
        if (!owner_thread_bound()) {
            ::glatter_log("GLATTER: owner thread not bound. Call glatter_bind_owner_to_current_thread() on the intended render thread.\n");
            abort();
        }
#endif
        return thread_ids_equal(current_thread_id(), owner_thread_id());
    }

    inline void bind_owner_to_current_thread()
    {
        owner_thread_id() = current_thread_id();
#if defined(GLATTER_REQUIRE_EXPLICIT_OWNER_BIND)
        owner_thread_bound() = true;
#endif
    }

}} // namespace glatter::detail

#else  // !header-only C++

GLATTER_EXTERN_C_BEGIN

#if !defined(GLATTER_HEADER_ONLY) || !defined(__cplusplus)
    /* Ownership policy:
     * - Default: first caller becomes owner (simple apps).
     * - Define GLATTER_REQUIRE_EXPLICIT_OWNER_BIND to force explicit binding,
     *   then call glatter_bind_owner_to_current_thread() on your render thread.
     */
#endif
#if defined(_WIN32)
    extern INIT_ONCE glatter_thread_once;
    extern DWORD     glatter_thread_id;
    extern int       glatter_owner_bound_explicitly;
    extern int       glatter_owner_thread_initialized;

    static BOOL CALLBACK glatter_set_owner_thread(PINIT_ONCE once, PVOID param, PVOID* context)
    {
        (void)once;
        (void)param;
        (void)context;
        /* Respect explicit owner binding: if the app already bound, do not overwrite. */
        if (glatter_owner_bound_explicitly || glatter_owner_thread_initialized) {
            return TRUE;
        }
        glatter_thread_id = GetCurrentThreadId();
        glatter_owner_thread_initialized = 1;
        return TRUE;
    }
#elif defined(__APPLE__) || defined(__unix__) || defined(__unix)
    extern pthread_once_t glatter_thread_once;
    extern pthread_t      glatter_thread_id;
    extern int            glatter_owner_bound_explicitly;
    extern int            glatter_owner_thread_initialized;

    static void glatter_set_owner_thread(void)
    {
        /* Respect explicit owner binding: if the app already bound, do not overwrite. */
        if (glatter_owner_bound_explicitly || glatter_owner_thread_initialized) {
            return;
        }
        glatter_thread_id = pthread_self();
        glatter_owner_thread_initialized = 1;
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
        /* Throttle: log this cross-thread warning at most once per thread. */
        static GLATTER_THREAD_LOCAL int glatter_warned_cross_thread = 0;
        if (!glatter_warned_cross_thread) {
            glatter_warned_cross_thread = 1;
            char* m = glatter_masprintf("GLATTER: Calling OpenGL from a different thread, in %s(%d)\n", file, line);
            glatter_log(m);
            free(m);
            /* glatter_log does not take ownership; caller frees 'm'. */
        }
    }

#elif defined(_WIN32)
    if (!InitOnceExecuteOnce(&glatter_thread_once, glatter_set_owner_thread, NULL, NULL)) {
        return;
    }
    DWORD current_thread = GetCurrentThreadId();
    if (current_thread != glatter_thread_id) {
        /* Throttle: log this cross-thread warning at most once per thread. */
        static GLATTER_THREAD_LOCAL int glatter_warned_cross_thread = 0;
        if (!glatter_warned_cross_thread) {
            glatter_warned_cross_thread = 1;
            char* m = glatter_masprintf("GLATTER: Calling OpenGL from a different thread, in %s(%d)\n", file, line);
            glatter_log(m);
            free(m);
        }
    }

#elif defined(__APPLE__) || defined(__unix__) || defined(__unix)
    pthread_once(&glatter_thread_once, glatter_set_owner_thread);
    pthread_t current_thread = pthread_self();
    if (!pthread_equal(current_thread, glatter_thread_id)) {
        /* Throttle: log this cross-thread warning at most once per thread. */
        static GLATTER_THREAD_LOCAL int glatter_warned_cross_thread = 0;
        if (!glatter_warned_cross_thread) {
            glatter_warned_cross_thread = 1;
            char* m = glatter_masprintf("GLATTER: Calling OpenGL from a different thread, in %s(%d)\n", file, line);
            glatter_log(m);
            free(m);
        }
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
    glatter_owner_thread_initialized = 1;
    glatter_owner_bound_explicitly = 1;
#elif defined(__APPLE__) || defined(__unix__) || defined(__unix)
    glatter_thread_id = pthread_self();
    glatter_owner_thread_initialized = 1;
    glatter_owner_bound_explicitly = 1;
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
    memset(&ret, 0, sizeof ret);
    const unsigned char *bytes = (const unsigned char*)obj;
    size_t cap = sizeof(ret.data);
    size_t pos = 0;

    if (cap == 0) {
        return ret;
    }

    ret.data[pos++] = '[';

    if (sz > 16) {
        sz = 16;
    }

    if (sz > 0) {
        int n = snprintf(ret.data + pos, cap - pos, "%02x", bytes[0]);
        if (n < 0) {
            n = 0;
        }
        if ((size_t)n >= cap - pos) {
            pos = cap - 1;
        }
        else {
            pos += (size_t)n;
            for (size_t i = 1; i < sz && pos < cap; ++i) {
                n = snprintf(ret.data + pos, cap - pos, " %02x", bytes[i]);
                if (n < 0) {
                    break;
                }
                if ((size_t)n >= cap - pos) {
                    pos = cap - 1;
                    break;
                }
                pos += (size_t)n;
            }
        }
    }

    if (pos < cap - 1) {
        ret.data[pos++] = ']';
    }
    else
    if (cap >= 2) {
        ret.data[cap - 2] = ']';
        pos = cap - 1;
    }

    ret.data[pos] = '\0';
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

#if defined(GLATTER_DEBUG) && !defined(GLATTER_RESOLVE_RETURNS_ZERO)
#   define GLATTER_RESOLVE_ABORT_ON_MISSING 1
#else
#   define GLATTER_RESOLVE_ABORT_ON_MISSING 0
#endif

/* Missing-symbol policy: in debug, abort to surface configuration errors early. */

#define GLATTER_RETURN_VALUE(return_or_not, rtype, value) \
    GLATTER_RETURN_VALUE_##return_or_not(rtype, value)

#define GLATTER_RETURN_VALUE_return(rtype, value) return (value)
#define GLATTER_RETURN_VALUE_(rtype, value)       return

#ifdef GLATTER_HEADER_ONLY

/* Thread-safe first use:
 * Resolution uses a single atomic CAS on the function pointer.
 * No locks / no call_once; safe in header-only and separate-TU builds.
 */
#define GLATTER_FBLOCK(return_or_not, family, cder, rtype, cconv, name, cargs, dargs)\
    typedef rtype (cconv *glatter_##name##_t) dargs;\
    static glatter_atomic(glatter_##name##_t) glatter_##name##_resolved = (glatter_##name##_t)0;\
    static inline rtype cconv glatter_##name dargs\
    {\
        glatter_##name##_t fn = GLATTER_ATOMIC_LOAD(glatter_##name##_resolved);\
        if (!fn) {\
            glatter_##name##_t resolved = (glatter_##name##_t)glatter_get_proc_address_##family(#name);\
            if (!resolved) {\
                /* Note: this message routes through the user-installed log handler\
                 * (default handler writes to stderr). Applications may replace it via\
                 * glatter_set_log_handler(...).\
                 */\
                if (GLATTER_RESOLVE_ABORT_ON_MISSING) {\
                    glatter_log_printf("GLATTER: missing '%s' (aborting in debug)\n", #name);\
                    abort();\
                }\
                glatter_log_printf("GLATTER: failed to resolve '%s'\n", #name);\
                GLATTER_RETURN_VALUE(return_or_not, rtype, (rtype)0);\
            }\
            glatter_##name##_t expected = (glatter_##name##_t)0;\
            if (!GLATTER_ATOMIC_CAS(glatter_##name##_resolved, expected, resolved)) {\
                resolved = GLATTER_ATOMIC_LOAD(glatter_##name##_resolved);\
            }\
            fn = resolved;\
        }\
        return_or_not fn cargs;\
    }

#else /* !GLATTER_HEADER_ONLY */

/* Thread-safe first use:
 * Resolution uses a single atomic CAS on the function pointer.
 * No locks / no call_once; safe in header-only and separate-TU builds.
 */
#define GLATTER_FBLOCK(return_or_not, family, cder, rtype, cconv, name, cargs, dargs)\
    cder rtype cconv name dargs;\
    typedef rtype (cconv *glatter_##name##_t) dargs;\
    static glatter_atomic(glatter_##name##_t) glatter_##name##_resolved = (glatter_##name##_t)0;\
    static rtype cconv glatter_##name##_dispatch dargs;\
    glatter_##name##_t glatter_##name = glatter_##name##_dispatch;\
    static rtype cconv glatter_##name##_dispatch dargs\
    {\
        glatter_##name##_t fn = GLATTER_ATOMIC_LOAD(glatter_##name##_resolved);\
        if (!fn) {\
            glatter_##name##_t resolved = (glatter_##name##_t)glatter_get_proc_address_##family(#name);\
            if (!resolved) {\
                /* Note: this message routes through the user-installed log handler\
                 * (default handler writes to stderr). Applications may replace it via\
                 * glatter_set_log_handler(...).\
                 */\
                if (GLATTER_RESOLVE_ABORT_ON_MISSING) {\
                    glatter_log_printf("GLATTER: missing '%s' (aborting in debug)\n", #name);\
                    abort();\
                }\
                glatter_log_printf("GLATTER: failed to resolve '%s'\n", #name);\
                GLATTER_RETURN_VALUE(return_or_not, rtype, (rtype)0);\
            }\
            glatter_##name##_t expected = (glatter_##name##_t)0;\
            if (!GLATTER_ATOMIC_CAS(glatter_##name##_resolved, expected, resolved)) {\
                resolved = GLATTER_ATOMIC_LOAD(glatter_##name##_resolved);\
            }\
            fn = resolved;\
        }\
        return_or_not fn cargs;\
    }

#endif /* GLATTER_HEADER_ONLY */

//==================

#include <stdarg.h>
#include <stdio.h>

/* Internal: format varargs and route to glatter_log(...) */
GLATTER_INLINE_OR_NOT
void glatter_vlog_line_(const char* prefix, const char* fmt, va_list ap)
{
    char buf[1024];
    /* Diagnostic logging: truncation is acceptable; vsnprintf prevents overflow. */
    int n = vsnprintf(buf, sizeof(buf), fmt ? fmt : "", ap);
    (void)n; /* truncation ignored intentionally */
    if (prefix && *prefix) glatter_log(prefix);
    glatter_log(buf);
}

GLATTER_INLINE_OR_NOT
void glatter_dbg_enter(const char* file, int line, const char* apiname,
                       const char* fmt, ...)
{
    glatter_pre_callback(file, line);
    glatter_log_printf("GLATTER: in '%s'(%d):\n", file, line);
    glatter_log_printf("GLATTER: %s", apiname);

    if (fmt && *fmt) {
        va_list ap; va_start(ap, fmt);
        glatter_vlog_line_("", fmt, ap);
        va_end(ap);
    }

    glatter_log("\n");
}

GLATTER_INLINE_OR_NOT
void glatter_dbg_return(const char* fmt, ...)
{
    glatter_log("GLATTER: returned ");
    if (fmt && *fmt) {
        va_list ap; va_start(ap, fmt);
        glatter_vlog_line_("", fmt, ap);
        va_end(ap);
    } else {
        glatter_log("(void)\n");
    }
}

#if defined(GLATTER_LOG_CALLS)

    /* Debug macros become thin function calls for debugger-friendly stepping */
    #define GLATTER_DBLOCK(file,line,name,printf_fmt,...) \
        glatter_dbg_enter((file),(line), #name, (printf_fmt), ##__VA_ARGS__)

    #define GLATTER_RBLOCK(...) \
        glatter_dbg_return(__VA_ARGS__)


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

    #if defined(GLATTER_EGL) && GLATTER_HAS_EGL_GENERATED_HEADERS
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

    #if defined(GLATTER_EGL) && GLATTER_HAS_EGL_GENERATED_HEADERS
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

#if defined(GLATTER_EGL) && GLATTER_HAS_EGL_GENERATED_HEADERS
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

#if defined(GLATTER_EGL) && GLATTER_HAS_EGL_GENERATED_HEADERS
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

#if defined(GLATTER_EGL) && GLATTER_HAS_EGL_GENERATED_HEADERS
    #include GLATTER_xstr(GLATTER_PDIR(GLATTER_PLATFORM_DIR)/glatter_EGL_e2s_def.h)
#endif

#if defined(GLATTER_WGL)
    #include GLATTER_xstr(GLATTER_PDIR(GLATTER_PLATFORM_DIR)/glatter_WGL_e2s_def.h)
#endif

#if defined(GLATTER_GLU)
    #include GLATTER_xstr(GLATTER_PDIR(GLATTER_PLATFORM_DIR)/glatter_GLU_e2s_def.h)
#endif

#if defined(GLATTER_EGL) && !GLATTER_HAS_EGL_GENERATED_HEADERS

GLATTER_INLINE_OR_NOT glatter_extension_support_status_EGL_t glatter_get_extension_support_EGL(void)
{
    glatter_extension_support_status_EGL_t status = {0};
    return status;
}

GLATTER_INLINE_OR_NOT const char* enum_to_string_EGL(GLATTER_ENUM_EGL e)
{
    (void)e;
    return "EGL_UNKNOWN";
}

#endif


#if defined(__llvm__) || defined (__clang__)
    #pragma clang diagnostic pop
#elif defined(__GNUC__)
    #pragma GCC diagnostic pop
#endif


GLATTER_EXTERN_C_END
