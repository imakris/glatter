#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GLATTER_CONFIG_H_DEFINED
/*
 * Provide a minimal configuration for the test. We skip the default
 * configuration header so we can compile without depending on any
 * platform-specific GL/EGL headers that may not be available.
 */

char* glatter_masprintf(const char* format, ...);

#define GLATTER_MASPRINTF_H
#include <glatter/glatter_def.h>

static const char* g_last_log_message = NULL;

static void test_log_handler(const char* str)
{
    g_last_log_message = str;
}

char* glatter_masprintf(const char* format, ...)
{
    (void)format;
    return NULL;
}

int main(void)
{
    glatter_set_log_handler(test_log_handler);

    const char* formatted = glatter_masprintf("glatter should fall back");
    const char* logged = glatter_log(formatted);

    if (formatted != NULL) {
        fprintf(stderr, "expected glatter_masprintf to fail\n");
        return 1;
    }

    if (logged != NULL) {
        fprintf(stderr, "glatter_log should return NULL when input is NULL\n");
        return 1;
    }

    if (g_last_log_message == NULL) {
        fprintf(stderr, "log handler was not invoked\n");
        return 1;
    }

    if (strcmp(g_last_log_message, "GLATTER: message formatting failed.\n") != 0) {
        fprintf(stderr, "unexpected fallback message: %s\n", g_last_log_message);
        return 1;
    }

    glatter_set_log_handler(NULL);

    if (glatter_log_handler() != glatter_default_log_handler) {
        fprintf(stderr, "NULL handler did not reset to default handler\n");
        return 1;
    }

    /* Smoke test to ensure the default handler can be invoked safely. */
    glatter_log("GLATTER: default handler smoke test.\n");

    return 0;
}
