#ifndef GLATTER_SOLO_H
#define GLATTER_SOLO_H

/* Enable header-only mode by default for C++ consumers unless explicitly disabled. */
#if defined(__cplusplus) && !defined(GLATTER_HEADER_ONLY) && !defined(GLATTER_NO_HEADER_ONLY)
#  define GLATTER_HEADER_ONLY 1
#endif

/* Mark that the default configuration should be used. */
#ifndef GLATTER_USER_CONFIGURED
#  define GLATTER_USER_CONFIGURED 0
#endif

#include <glatter/glatter.h>

#endif /* GLATTER_SOLO_H */
