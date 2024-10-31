#include <stdio.h>


int main(int argc, char * argv[]){
    FILE * test_file = fopen("test.txt", "r");
    if (test_file == NULL) {
       printf("Error opening file");
       return 1;
   }

    char ch;
    while ((ch = fgetc(test_file)) != EOF) {
        printf("%c",ch);
    }

    fclose(test_file);
    test_file = fopen("test.txt", "a");
    const char *log_entry = "simple append to a file \n";
    fprintf(test_file, "%s", log_entry);


    fclose(test_file);
    return 0;
}