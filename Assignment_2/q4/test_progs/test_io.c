// Test disk I/O limit
#include <stdio.h>
#include <stdlib.h>

int main() {
    FILE *fp;
    char buffer[1024 * 1024 * 2]; // 2MB buffer
    
    fp = fopen("test.dat", "w");
    if (fp == NULL) {
        perror("fopen");
        return 1;
    }
    
    // Try to write 2MB (over limit)
    fwrite(buffer, 1, sizeof(buffer), fp);
    fclose(fp);
    
    return 0;
}
