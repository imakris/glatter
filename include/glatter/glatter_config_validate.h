#ifndef GLATTER_CONFIG_VALIDATE_H
#define GLATTER_CONFIG_VALIDATE_H

#if defined(GLATTER_GLX) && defined(GLATTER_WGL)
#  error "Choose exactly one desktop WSI: GLX or WGL (or use AUTO)."
#endif

#if GLATTER_GLES_VERSION && !defined(GLATTER_EGL)
#  error "GLES requires EGL. Set GLATTER_PLATFORM to EGL or AUTO."
#endif

#endif /* GLATTER_CONFIG_VALIDATE_H */
