#include <glatter/glatter.h>
#include <stdio.h>

int main() {
    glatter_set_wsi(GLATTER_WSI_AUTO_VALUE);
    printf("Glatter integration test compiled and linked successfully.\n");
    
    if (glatter_glGetString) {
        printf("A glatter function pointer is available.\n");
    }

    return 0;
}