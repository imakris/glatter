# glatter — OpenGL‑family loader & tracer (zero‑config)

**glatter** is a lightweight loader and tracer for OpenGL‑family APIs (GL, GLX, WGL, EGL, GLES, optional GLU). It resolves function pointers on demand and, when enabled, logs calls, checks for errors, and warns on cross‑thread usage. Works in C and C++ with sensible defaults.

---

## 1) Zero‑config quick start

### C++ (header‑only)
```cpp
#include <glatter/glatter_solo.h>
// Use GL entry points as usual
glClear(GL_COLOR_BUFFER_BIT);
```

### C (compiled translation unit)
```c
#include <glatter/glatter.h>
/* add src/glatter.c to your build */

// Use GL entry points as usual
glClear(GL_COLOR_BUFFER_BIT);
```

**Do not include the system GL headers** (e.g., `GL/gl.h`, `EGL/egl.h`) yourself; glatter pulls the right headers for you.

Header‑only is enabled automatically for C++ when including `glatter_solo.h` or `glatter.h`. To use the compiled TU from C++ as well, define `GLATTER_NO_HEADER_ONLY` **before** including the header and add `src/glatter.c` to your build.

---

## 2) What “auto” config does

Glatter auto‑selects wrappers and a provider based on the host OS, and it can try multiple backends at runtime if needed:

- **Windows:** WGL + GL
- **Linux/BSD:** GLX + GL (with transparent fallback to EGL + GLES)
- **Android/Emscripten:** EGL + GLES (defaults to GLES 3.2 when headers support it)

You can steer resolution when necessary:
```c
/* C or C++ */
glatter_set_provider(GLATTER_PROVIDER_EGL);  /* or GLATTER_PROVIDER_GLX, GLATTER_PROVIDER_WGL */
/* or via environment: GLATTER_PROVIDER=egl|glx|wgl */
```

---

## 3) Public API (practical summary)

- `void glatter_bind_owner_to_current_thread(void);`  — Bind the diagnostic “owner thread”. Header‑only mode binds on first use; calling this early yields clearer warnings.
- `void glatter_set_log_handler(void (*handler)(const char*));`  — Install a custom logger. On platforms without atomics, set this during single‑threaded init.
- `void* glatter_get_proc_address(const char* name);`  — Unified proc resolver; you rarely need it directly.
- `void glatter_set_provider(int provider);` / `int glatter_get_provider(void);`  — Override or inspect the runtime provider (`AUTO`, `WGL`, `GLX`, `EGL`).
- `glatter_get_extension_support_*()` — Return a struct of per‑extension booleans for GL/GLX/EGL/WGL (see §6).
- `const char* enum_to_string_*()` — Human‑readable names for error/status enums across GL, GLX, WGL, EGL, GLU.

---

## 4) Tracing & error reporting

Two independent, opt‑in layers:
- `GLATTER_LOG_CALLS` — log every wrapped call (name, arguments, return value).
- `GLATTER_LOG_ERRORS` — log only API errors.

If neither is defined, **debug builds default to error‑only logging** (i.e., when `NDEBUG` is not defined). You can direct logs to your sink:
```c
void my_log_sink(const char* msg) { /* route to your logger */ }
int main() {
    glatter_set_log_handler(my_log_sink);
}
```

Note: ARB/KHR debug output still requires a debug context and `GL_DEBUG_OUTPUT_SYNCHRONOUS`; glatter’s error checks work independently of those.

---

## 5) Thread ownership checks

Glatter tracks an “owner thread” and warns when a different thread makes GL calls.
- **Header‑only C++:** the first touching thread becomes owner. Optionally call `glatter_bind_owner_to_current_thread()` during init. If you define `GLATTER_REQUIRE_EXPLICIT_OWNER_BIND`, the library aborts if you forget to bind before first use.
- **Compiled C/C++:** the owner is captured once on first use; later calls from other threads are reported.

This is diagnostic only; glatter does not serialize or block.

---

## 6) Checking extension support

Query support once and then use fast, per‑extension booleans:
```c
/* Example for core GL */
auto s = glatter_get_extension_support_GL();
/* Inspect the field matching your extension (see generated *_ges_decl.h): */
/* if (s.GL_ARB_debug_output) { ... } */
```
The first call performs the underlying string queries and caches the result for subsequent checks.

---

## 7) GLX Xlib error handler

When using the GLX provider, glatter installs a tiny X error handler the first time GLX is touched, to surface common GLX/Xlib issues. If you already manage your own handler (or need to install one after `XInitThreads()`), define:
```c
#define GLATTER_DO_NOT_INSTALL_X_ERROR_HANDLER
```

---

## 8) Windows UNICODE vs MBCS

No special switches required. The generator assumes UNICODE; non‑UNICODE (`MBCS`) builds are detected and `TCHAR` is mapped accordingly for generated prototypes where relevant.

---

## 9) Symbol resolution details (for the curious)

Each function is resolved once and cached.
- **WGL:** `wglGetProcAddress`, then `GetProcAddress(opengl32.dll)`
- **GLX:** `glXGetProcAddress(ARB)`, then `dlsym(libGL.so.1|libGL.so)`
- **EGL/GLES:** `eglGetProcAddress`, then `dlsym(libEGL.so.1|.so)`, then GLES sonames (`libGLESv3|v2|v1_CM`)
- **GLU (optional):** `glu32.dll` on Windows, `libGLU.so.1|libGLU.so` on Linux

---

## 10) Advanced configuration

Two headers are meant for power users:
- **`glatter_config.h`** — feature/platform switches. Define `GLATTER_USER_CONFIGURED` before including glatter to opt out of the zero‑config defaults and set your own `GLATTER_*` macros.
- **`glatter_platform_headers.h`** — lists the API headers glatter should use. If you edit this, re‑run the generator (`python include/glatter/glatter.py`).

### Regenerating headers (optional)
1) Place target API headers under `include/glatter/headers/`.
2) Add the bundle to `glatter_platform_headers.h` (one `#include` per line, no trailing comments).
3) Run: `python include/glatter/glatter.py`.

If `GLATTER_HAS_EGL_GENERATED_HEADERS` is off for your target, EGL/GLES helpers are unavailable until you generate the headers; the library still builds.

---

## 11) Integration notes

- Include only `<glatter/...>` in your sources.
- **C++:** header‑only by default; to use compiled mode define `GLATTER_NO_HEADER_ONLY` and add `src/glatter.c`.
- **Linking:**
  - Windows: link `opengl32` (and `EGL`/`GLES` DLLs if you use them).
  - Linux/BSD: link `GL`, `X11`, `pthread`, `dl` (and `EGL`/`GLES` if you use them).

---

## 12) CMake examples

**C app (compiled TU):**
```cmake
add_library(glatter src/glatter.c)
target_include_directories(glatter PUBLIC include)
if(WIN32)
  target_link_libraries(glatter PUBLIC opengl32)
else()
  find_package(X11 REQUIRED)
  find_package(Threads REQUIRED)
  target_link_libraries(glatter PUBLIC GL Threads::Threads X11::X11 dl)
endif()

add_executable(my_app src/main.c)
target_link_libraries(my_app PRIVATE glatter)
```

**C++ app (header‑only):**
```cmake
add_executable(my_app src/main.cpp)
target_include_directories(my_app PRIVATE include)
if(WIN32)
  target_link_libraries(my_app PRIVATE opengl32)
else()
  target_link_libraries(my_app PRIVATE GL Threads::Threads X11::X11 dl)
endif()
```

---

## 13) Troubleshooting

- **“Failed to resolve …”** — ensure a current context and that GL/EGL/GLES libraries are visible on your platform.
- **Cross‑thread warnings** — call `glatter_bind_owner_to_current_thread()` on the render thread, or enable strict binding with `GLATTER_REQUIRE_EXPLICIT_OWNER_BIND`.
- **GLX error spam** — define `GLATTER_DO_NOT_INSTALL_X_ERROR_HANDLER` and install your own after X threading init.
- **Missing EGL/GLES generated headers** — builds still succeed; EGL/GLES helpers are unavailable until you generate headers.

---

## 14) License

BSD‑2‑Clause (Simplified). See the header prologs for the full text.

