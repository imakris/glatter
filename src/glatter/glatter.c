#include <glatter/glatter_config.h>

#ifdef GLATTER_HEADER_ONLY
#error "Do not compile glatter.c when GLATTER_HEADER_ONLY is defined."
#endif

#include <glatter/glatter_platform_headers.h>

#if defined(_WIN32)
INIT_ONCE glatter_thread_once = INIT_ONCE_STATIC_INIT;
DWORD glatter_thread_id = 0;
#elif defined(__APPLE__) || defined(__unix__) || defined(__unix)
pthread_once_t glatter_thread_once = PTHREAD_ONCE_INIT;
pthread_t glatter_thread_id;
#else
#error "Unsupported platform"
#endif

#include <glatter/glatter_def.h>
