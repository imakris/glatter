#ifndef GLATTER_CONFIG_USER_H
#define GLATTER_CONFIG_USER_H

/* == APIs (choose what you need) == */
#define GLATTER_GL                 1   /* 1=enable, 0=disable */
#define GLATTER_GLU                0
/* GLES: set version. Allowed: 0, 11, 20, 30, 31, 32 */
#define GLATTER_GLES_VERSION       0   /* 0 = no GLES */

/* == Platform / WSI selection == */
#define GLATTER_PLATFORM_AUTO      0
#define GLATTER_PLATFORM_WGL       1
#define GLATTER_PLATFORM_GLX       2
#define GLATTER_PLATFORM_EGL       3

#ifndef GLATTER_PLATFORM
#  define GLATTER_PLATFORM         GLATTER_PLATFORM_AUTO
#endif

/* == Logging == */
#define GLATTER_LOG_ERRORS         1
#define GLATTER_LOG_CALLS          0

/* == Threading / Ownership == */
#define GLATTER_REQUIRE_EXPLICIT_OWNER_BIND 0

/* == X11 == */
#define GLATTER_INSTALL_X_ERROR_HANDLER    1   /* 0 to opt-out */

#endif /* GLATTER_CONFIG_USER_H */
