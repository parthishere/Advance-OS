// Test disk I/O limit
#include <stdio.h>
#include <stdlib.h>

int main() {
    FILE *fp;
    char buffer[1024 * 1024 * 4]; // 4MB buffer
    
    fp = fopen("test.dat", "w");
    if (fp == NULL) {
        printf("error fopen\n");
        return 1;
    }
    
    // Try to write 4MB (over limit)
    fwrite(buffer, 1, sizeof(buffer), fp);
    fclose(fp);
    
    return 0;
}
