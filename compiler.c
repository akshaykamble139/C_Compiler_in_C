#include <stdio.h>
#include <string.h>
#include <stdlib.h>


int main(int argc, char* argv[]){
    char FILE_PATH[50] = "./Files/";
    

    if (argc == 2){

        int i;

        int f = strlen(FILE_PATH);

        for (i = 0; argv[1][i] != '\0'; i++) {    
            FILE_PATH[i + f] = argv[1][i];
        }

        FILE_PATH[i + f] = '\0';
    }
    else{
        printf("please enter only one file name");
        return 1;
    }

    printf("file name: %s", FILE_PATH);
}