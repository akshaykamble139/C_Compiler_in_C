#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define INITIAL_CAPACITY 10
#define LINE_LENGTH 256

char* FILE_PATH = NULL;
FILE *file;
char line[LINE_LENGTH];
char **lines = NULL;
size_t capacity = INITIAL_CAPACITY;
size_t size = 0;

char* get_C_File_name(int argc, char* argv[]);
int read_file_line_by_line();


int main(int argc, char* argv[]){
    FILE_PATH = get_C_File_name(argc,argv);

    if (FILE_PATH == NULL) {
        return 1;
    }   

    // printf("file name: %s\n", FILE_PATH);

    if (read_file_line_by_line()){
        return 1;
    }

    for (size_t i = 0; i < size; i++) {
        printf("%d %s\n", strlen(lines[i]), lines[i]);
        free(lines[i]);  
    }

    free(lines);

    return 0;

}

char* get_C_File_name(int argc, char* argv[]){
    if (argc == 2){

        int i;

        int d = strlen(argv[1]);

        FILE_PATH = malloc((8+d) * sizeof(char));
        char path[] = "./Files/";

        for (i = 0;i<8;i++){
            FILE_PATH[i] = path[i];
        }

        for (; argv[1][i-8] != '\0'; i++) {    
            FILE_PATH[i] = argv[1][i-8];
        }

        FILE_PATH[i] = '\0';
    }
    else{
        printf("please enter only one file name");
        return NULL;
    }

    return FILE_PATH;
}

int read_file_line_by_line()
{
    int retFlag = 1;
    file = fopen(FILE_PATH, "r");

    if (file == NULL)
    {
        perror("Error opening file");
        return 1;
    }

    lines = malloc(capacity * sizeof(char *));
    if (lines == NULL)
    {
        perror("Error allocating memory");
        fclose(file);
        return 1;
    }

    while (fgets(line, sizeof(line), file))
    {

        line[strcspn(line, "\n")] = 0;

        if (size >= capacity)
        {
            capacity *= 2;
            char **new_lines = realloc(lines, capacity * sizeof(char *));
            if (new_lines == NULL)
            {
                perror("Error reallocating memory");
                for (size_t i = 0; i < size; i++)
                {
                    free(lines[i]);
                }
                free(lines);
                fclose(file);
                return 1;
            }
            lines = new_lines;
        }

        lines[size] = malloc(strlen(line) + 1);
        if (lines[size] == NULL)
        {
            perror("Error allocating memory for line");
            for (size_t i = 0; i < size; i++)
            {
                free(lines[i]);
            }
            free(lines);
            fclose(file);
            return 1;
        }
        strcpy(lines[size], line);
        size++;
    }

    fclose(file);
    retFlag = 0;
    return retFlag;
}
