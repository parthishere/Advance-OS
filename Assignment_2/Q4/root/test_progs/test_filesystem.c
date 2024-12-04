#include <stdio.h>

int main() {
    FILE *fp = fopen("/etc/passwd", "r");
    if (fp == NULL) {
        printf("File access denied (expected)\n");
        return 1;
    }
    fclose(fp);
    return 0;
}