#ifndef GLATTER_MASPRINTF_H
#define GLATTER_MASPRINTF_H


#include <stdio.h>  /* needed for vsnprintf    */
#include <stdlib.h> /* needed for malloc, free */
#include <stdarg.h> /* needed for va_*         */

/*
 * vscprintf:
 * MSVC implements this as _vscprintf, thus we just 'symlink' it here
 * GNU-C-compatible compilers do not implement this, thus we implement it here
 */
#ifdef _MSC_VER
#define vscprintf _vscprintf
#endif

#ifndef GLATTER_MASPRINTF_INLINE
# ifdef GLATTER_INLINE_OR_NOT
#  define GLATTER_MASPRINTF_INLINE static GLATTER_INLINE_OR_NOT
# elif defined(_MSC_VER)
#  define GLATTER_MASPRINTF_INLINE static __inline
# elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#  define GLATTER_MASPRINTF_INLINE static inline
# else
#  define GLATTER_MASPRINTF_INLINE static
# endif
#endif

#ifdef __GNUC__
GLATTER_MASPRINTF_INLINE int vscprintf(const char* format, va_list ap);
#endif

GLATTER_MASPRINTF_INLINE char* glatter_mvasprintf(const char* format, va_list ap);
GLATTER_MASPRINTF_INLINE char* glatter_masprintf(const char* format, ...);

#ifdef __GNUC__
GLATTER_MASPRINTF_INLINE int vscprintf(const char* format, va_list ap)
{
    va_list ap_copy;
    va_copy(ap_copy, ap);
    int retval = vsnprintf(NULL, 0, format, ap_copy);
    va_end(ap_copy);
    return retval;
}
#endif



GLATTER_MASPRINTF_INLINE char* glatter_mvasprintf(const char* format, va_list ap)
{
    int len = vscprintf(format, ap);
    if (len == -1)
        return 0;
    char* str = (char*)malloc((size_t)len + 1);
    if (!str)
        return 0;
    int val = vsnprintf(str, len + 1, format, ap);
    if (val == -1) {
        free(str);
        return 0;
    }
    return str;
}


GLATTER_MASPRINTF_INLINE char* glatter_masprintf(const char* format, ...)
{
    va_list ap;
    va_start(ap, format);
    char* retval = glatter_mvasprintf(format, ap);
    va_end(ap);
    return retval;
}

#endif // GLATTER_MASPRINTF_H
