// Test memory limit
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main() {
    char *ptr;
    size_t size = 5 * 1024 * 1024; // Try to allocate 3MB (over limit)
    
    int letssee[5*1024*1024];
    ptr = malloc(size);
    if (ptr == NULL) {
        printf("Memory allocation failed (expected)\n");
        return 1;
    }
    
    memset(ptr, 0, size);
    free(ptr);
    return 0;
}