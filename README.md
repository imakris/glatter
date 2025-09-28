# glatter — OpenGL‑family loader & tracer

A practical loader and tracer for GL‑family APIs (GL, GLX, WGL, EGL, GLES, optional GLU). Resolves symbols on first use, lets you log calls or just errors, and warns on cross‑thread usage. Works in C and C++ with sensible defaults.

---

## Technical summary

* **Integration:** header‑only (C++) or compiled translation unit (C/C++).
* **WSI detection:** auto‑detects WGL/GLX/EGL with optional runtime override.
* **On‑demand symbols:** function pointers are resolved on first use to minimize startup work.
* **Diagnostics:** opt‑in call/error logging; in debug builds (when `NDEBUG` is not defined) errors are logged by default. Messages go to stdout/stderr; install a custom handler only if you want to redirect.
* **Utilities:** generated extension flags (e.g., `glatter_GL_ARB_vertex_array_object`) and `enum_to_string_*()` helpers.

---

## Choose your integration mode

Pick **one** of the two ways to use glatter:

### 1) Header‑only (C++ only)

```cpp
#include <glatter/glatter_solo.h>
// Use GL entry points as usual
glClear(GL_COLOR_BUFFER_BIT);
```

For header-only usage, include `glatter/glatter_solo.h`. This tiny wrapper defines
`GLATTER_HEADER_ONLY` and then includes the main `glatter.h` for you.

### 2) Compiled C translation unit (C or C++)

```c
#include <glatter/glatter.h>
/* Add src/glatter/glatter.c to your build */

// Use GL entry points as usual
glClear(GL_COLOR_BUFFER_BIT);
```

**Note:** Do not include system GL headers yourself (e.g., `GL/gl.h`, `EGL/egl.h`). Glatter chooses the right ones for you.

---

## Quick start

1. Add the `include/` directory to your compiler’s include paths.
2. Choose either **header‑only** (C++) or **compiled TU** (C/C++), as shown above.
3. Link platform libraries (see Integration notes).
4. Optional: install a custom log sink only if you want to redirect messages away from stdout/stderr.

---

## Window System Interface (WSI) selection (auto and overrides)

Glatter auto‑selects the appropriate Window System Interface (WSI) for your platform.

Optional WSI override (function call or env var):

```c
/* Optional WSI override (defaults are auto‑detected).
   Alternatively set: GLATTER_WSI=egl|glx|wgl */

glatter_set_wsi(GLATTER_WSI_EGL); /* or WGL, GLX, AUTO */
```

**Thread-safety:** When `GLATTER_WSI=AUTO`, glatter decides the WSI exactly once at first use.
This decision is synchronized so concurrent first calls cannot pick different backends.
In header-only builds the decision gate is per translation unit; use the compiled TU mode
to enforce a single process-wide decision.

## Typical single‑context app

```cpp
#include <glatter/glatter_solo.h> // header‑only C++ header; for C/C++ compiled TU use <glatter/glatter.h> and compile src/glatter/glatter.c

int main() {
    // Create and make a GL context current here
    // ...

    // Gate once on a requirement (example: VAOs)
    if (!glatter_GL_ARB_vertex_array_object) {
        // Extension not available → bail out or use a fallback
        return 1;
    }

    // Safe to use the corresponding functions
    GLuint vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glClear(GL_COLOR_BUFFER_BIT);
    return 0;
}
```

## API summary (essentials)

* **WSI override/inspect**: `glatter_set_wsi(GLATTER_WSI_*)`, `glatter_get_wsi()` (APIs use the term "Window System Interface (WSI)").
* **Extension flags**: test generated flags like `glatter_GL_ARB_vertex_array_object` once the context is current.
* **Enum names**: `enum_to_string_*()` for readable GL/GLX/WGL/EGL/GLU enums.

Notes: Diagnostics and multi‑context thread checks are covered under **Tracing & diagnostics**. Low‑level entry‑point helpers are documented under **Advanced** and are rarely needed.

## Tracing & error logging

By default, messages go to stdout/stderr. Install a custom log handler only to redirect or integrate with your own logging.

Two independent, opt‑in layers:

* `GLATTER_LOG_CALLS` Log every wrapped call (name, args, return).
* `GLATTER_LOG_ERRORS` Log only API errors.

If neither is defined, **debug builds default to error‑only logging** (i.e., when `NDEBUG` is not defined).

Redirect output to your sink:

```c
void my_log_sink(const char* msg) { /* route to your logger */ }
int main() {
    glatter_set_log_handler(my_log_sink);
}
```

> Note: ARB/KHR debug output still needs a debug context; glatter’s error checks work independently.

For WGL wrappers, glatter sets `SetLastError(0)` immediately before the call so the subsequent
`GetLastError()` check reflects that call. The log message indicates if the error code may be stale.

---

## Thread ownership checks

Glatter tracks an "owner thread" and warns when calls come from a different thread.

* **Header‑only C++:** first touching thread becomes owner. Call `glatter_bind_owner_to_current_thread()` early if you want explicit control. Define `GLATTER_REQUIRE_EXPLICIT_OWNER_BIND` to require an explicit bind, otherwise the library aborts on first use without binding.
* **Compiled C/C++:** the owner is captured on first use; later calls from other threads are reported.

This is diagnostic only. Glatter does not serialize or block.

---

## Extension requirements

Use the generated boolean flags `glatter_<EXTENSION_NAME>` after a context is current. Example (VAOs):

```c
if (!glatter_GL_ARB_vertex_array_object) {
    /* Report and exit early or use a fallback */
    return 1;
}

GLuint vao = 0;
glGenVertexArrays(1, &vao);
glBindVertexArray(vao);
```

---

## GLX Xlib error handler

When using the GLX WSI, glatter installs a small X error handler the first time GLX is touched, to surface common GLX/Xlib issues. To opt out and install your own handler (for example after `XInitThreads()`), define:

```c
#define GLATTER_DO_NOT_INSTALL_X_ERROR_HANDLER
```

---

## Integration notes

* Include only headers under `<glatter/...>`.
* **C++ header‑only:** include `glatter/glatter_solo.h`.
* **C or C++ compiled TU:** include `glatter/glatter.h` and add `src/glatter/glatter.c` to your build.
* **Linking**

  * Windows: link against `opengl32` (plus `EGL`/`GLES` DLLs if you use them).
  * Linux/BSD: link against `GL`, `X11`, `pthread`, `dl` (plus `EGL`/`GLES` if you use them).

---

## CMake snippets

**C app (compiled TU):**

```cmake
add_library(glatter src/glatter/glatter.c)
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

## Advanced configuration

### Windows character encoding

Handles UNICODE and MBCS builds. The generator assumes UNICODE by default; on non-UNICODE builds, the `GLATTER_WINDOWS_MBCS` switch is automatically set unless you define it yourself. This keeps TCHAR handling correct without extra setup.

### Regenerating headers (optional)

Two headers are meant for power users:

* `glatter_config.h` Feature/platform switches. Define `GLATTER_USER_CONFIGURED` and set your own `GLATTER_*` macros to opt out of the defaults.
* `glatter_platform_headers.h` The list of API headers glatter should use. If you edit this, re‑run the generator.

1. Place target API headers under `include/glatter/headers/`.
2. Add the bundle to `glatter_platform_headers.h` (one `#include` per line, no trailing comments).
3. Run either:
   * `python include/glatter/glatter.py`
   * `cd include/glatter && python glatter.py`

If `GLATTER_HAS_EGL_GENERATED_HEADERS` is off for your target, EGL/GLES helpers are unavailable until you generate the headers; the library still builds.

---

## Troubleshooting

* **“Failed to resolve …”** Ensure a current context and that GL/EGL/GLES libraries are visible on your platform.
* **Cross‑thread warnings** Call `glatter_bind_owner_to_current_thread()` on the render thread, or enable strict binding with `GLATTER_REQUIRE_EXPLICIT_OWNER_BIND`.
* **GLX error spam** Define `GLATTER_DO_NOT_INSTALL_X_ERROR_HANDLER` and install your own after X threading init.
* **Missing EGL/GLES generated headers** Builds still succeed; EGL/GLES helpers are unavailable until you generate headers.

---

## License

BSD‑2‑Clause (Simplified). See header prologs for the full text.
