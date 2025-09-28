#ifndef GLATTER_CONFIG_H_DEFINED
#define GLATTER_CONFIG_H_DEFINED

/* 1) Pull user knobs if present locally; else fall back to installed copy */
#if defined(__has_include)
#  if __has_include("glatter_config_user.h")
#    include "glatter_config_user.h"
#  else
#    include <glatter/glatter_config_user.h>
#  endif
#else
#  include <glatter/glatter_config_user.h>
#endif

/* 2) Internal mapping + defaults + validation */
#include <glatter/glatter_config_internal.h>
#include <glatter/glatter_config_validate.h>

#endif /* GLATTER_CONFIG_H_DEFINED */
