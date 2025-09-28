#ifndef GLATTER_CONFIG_VALIDATE_H
#define GLATTER_CONFIG_VALIDATE_H

/* Ensure platform constants were not altered downstream */
#if (GLATTER_PLATFORM_AUTO != 0) || (GLATTER_PLATFORM_WGL != 1) || \
    (GLATTER_PLATFORM_GLX  != 2) || (GLATTER_PLATFORM_EGL != 3)
# error "Do not modify GLATTER_PLATFORM_* constants; set GLATTER_PLATFORM only."
#endif

#endif /* GLATTER_CONFIG_VALIDATE_H */
