#ifndef __GLATTER_CONFIG_H__
#define __GLATTER_CONFIG_H__

#define GLATTER_GL

#if defined(_WIN32)
	#define GLATTER_WGL
#elif defined(__linux__)
	#define GLATTER_GLX
	//#define GLATTER_DO_NOT_INSTALL_X_ERROR_HANDLER
#endif

//#define GLATTER_LOG

#endif
