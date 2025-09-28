#ifndef GLATTER_CONFIG_H_DEFINED
#define GLATTER_CONFIG_H_DEFINED

/* 0) Immutable choice constants */
#include <glatter/glatter_config_choices.h>

/* 1) User-facing knobs (prefer local copy if present) */
#if defined(__has_include)
# if __has_include("glatter_config_user.h")
#  include "glatter_config_user.h"
# else
#  include <glatter/glatter_config_user.h>
# endif
#else
# include <glatter/glatter_config_user.h>
#endif

/* 2) Internal defaults + mapping logic */
#include <glatter/glatter_config_internal.h>

#endif /* GLATTER_CONFIG_H_DEFINED */
