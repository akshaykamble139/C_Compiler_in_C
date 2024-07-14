#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define INITIAL_CAPACITY 10
#define LINE_LENGTH 256
#define INITIAL_TOKEN_CAPACITY 100

typedef enum {
    TOKEN_IDENTIFIER,
    TOKEN_KEYWORD,
    TOKEN_INTEGER,
    TOKEN_FLOAT,
    TOKEN_STRING,
    TOKEN_OPERATOR,
    TOKEN_PUNCTUATOR,
    TOKEN_EOF,
    TOKEN_PREPROCESSOR_DIRECTIVE
} TokenType;

typedef struct {
    TokenType type;
    char* lexeme;
    int line;
    int column;
} Token;

typedef struct {
    char** input;
    size_t current_pos;
    int line;
    int column;
    int on_a_new_line_char;
} Lexer;

char* FILE_PATH = NULL;
FILE *file;
char line[LINE_LENGTH];
char **lines = NULL;
size_t capacity = INITIAL_CAPACITY;
size_t tokens_capacity = INITIAL_TOKEN_CAPACITY;
size_t size = 0;
size_t tokens_num = 0;

Token** token_list;

char* get_C_File_name(int argc, char* argv[]);
int read_file_line_by_line();
int get_token_list(Lexer* lexer);

Lexer* init_lexer(char **l);
char peek_char(Lexer* lexer);
char read_char(Lexer* lexer);
void skip_comment(Lexer* lexer);
Token* create_token(TokenType type, char* lexeme, int line, int column);
Token* get_next_token(Lexer* lexer);
Token* read_identifier(Lexer* lexer);
int is_keyword(char* lexeme);

void free_resources(Lexer* lexer);


int main(int argc, char* argv[]){
    FILE_PATH = get_C_File_name(argc,argv);

    if (FILE_PATH == NULL) {
        free(FILE_PATH);
        return 1;
    }   

    if (read_file_line_by_line()){
        free_resources(NULL);
        return 1;
    }

    Lexer* lexer = init_lexer(lines);

    if (get_token_list(lexer)) {
        free_resources(lexer);
        return 1;
    }

    free_resources(lexer);

    return 0;

}

void free_resources(Lexer* lexer) {

    if (lexer != NULL) {
        free(lexer);
    }

    free(FILE_PATH);

    if (size > 0) {
        for (size_t i = 0; i < size; i++) {
            free(lines[i]);  
        }
        free(lines);
    }

    if (tokens_num > 0) {
        for (size_t i = 0; i < tokens_num; i++) {
            free(token_list[i]->lexeme);
            free(token_list[i]);  
        }
        free(token_list);
    }
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

        line[strcspn(line, "\n")+1] = 0;

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

int get_token_list(Lexer* lexer) {
    token_list = (Token**) malloc(tokens_capacity * sizeof(Token *));
    if (token_list == NULL)
    {
        perror("Error allocating memory");
        free(token_list);
        return 1;
    }

    token_list[tokens_num] = (Token*) malloc(sizeof(Token));
    token_list[tokens_num] = get_next_token(lexer);
    while (token_list[tokens_num]->type != TOKEN_EOF) {
        printf("inside while\n");
        printf("Token: type=%d, lexeme='%s', line=%d, column=%d\n",
               token_list[tokens_num]->type, token_list[tokens_num]->lexeme, 
               token_list[tokens_num]->line, token_list[tokens_num]->column);
        
        if (tokens_num >= tokens_capacity) {
            tokens_capacity *= 2;
            Token **new_list = (Token**)realloc(token_list, tokens_capacity * sizeof(Token *));
            if (new_list == NULL)
            {
                perror("Error reallocating memory");
                free_resources(lexer);
                return 1;
            }
            token_list = new_list;
        }
        token_list[tokens_num+1] = (Token*) malloc(sizeof(Token));
        token_list[tokens_num+1] = get_next_token(lexer);

        if (token_list[tokens_num]->lexeme == "\n") {
            printf("bruh\n");
        }

        tokens_num++;

        if (tokens_num > 50) {
            break;
        }
    }

    printf("%d\n",token_list[tokens_num]->type != TOKEN_EOF);

    return 0;
}

Lexer* init_lexer(char** program_lines) {
    Lexer* lexer = (Lexer*)malloc(sizeof(Lexer));
    lexer->input = program_lines;
    lexer->line = 1;
    lexer->current_pos = 0;
    lexer->column = 1;
    lexer->on_a_new_line_char = 0;
    return lexer;
}

char peek_char(Lexer* lexer) {
    if (lexer->line > size) {
        return '\0';
    }
    return lexer->input[lexer->line - 1][lexer->current_pos];
}

char read_char(Lexer* lexer) {
    char c = peek_char(lexer);
    printf("inside read_char %c %d %d %d\n", c, lexer->line, lexer->column, lexer->on_a_new_line_char);
    if (lexer->line <= size && c == '\n') {
        printf("inside if read_char %d\n", lexer->line);
        lexer->on_a_new_line_char = 0;
        lexer->current_pos = 0;
        lexer->line++;
        lexer->column = 1;
        c = peek_char(lexer);
    }
    
    lexer->current_pos++;
    lexer->column++;
    lexer->on_a_new_line_char = c == '\n';
    
    return c;
}

Token* create_token(TokenType type, char* lexeme, int line, int column) {
    Token* token = (Token*)malloc(sizeof(Token));
    token->type = type;
    token->lexeme = strdup(lexeme);
    token->line = line;
    token->column = column;
    return token;
}

void skip_comment(Lexer* lexer) {
    printf("inside skip comment\n");
    char c = read_char(lexer);
    if (c == '/') {
        while (peek_char(lexer) != '\n' && peek_char(lexer) != '\0') {
            read_char(lexer);
        }
    } else if (c == '*') {
        while (1) {
            c = read_char(lexer);
            if (c == '*' && peek_char(lexer) == '/') {
                read_char(lexer);
                break;
            }
            if (c == '\0') {
                fprintf(stderr, "Error: Unterminated multi-line comment\n");
                break;
            }
        }
    }
}

int is_keyword(char* lexeme) {
    static const char* keywords[] = {
        "auto", "break", "case", "char", "const", "continue", "default", "do",
        "double", "else", "enum", "extern", "float", "for", "goto", "if",
        "int", "long", "register", "return", "short", "signed", "sizeof", "static",
        "struct", "switch", "typedef", "union", "unsigned", "void", "volatile", "while"
    };
    static const int num_keywords = sizeof(keywords) / sizeof(keywords[0]);
    
    for (int i = 0; i < num_keywords; i++) {
        if (strcmp(lexeme, keywords[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

Token* read_identifier(Lexer* lexer) {
    int start_pos = lexer->current_pos;
    int start_line = lexer->line;
    int start_column = lexer->column;
    
    while (isalnum(peek_char(lexer)) || peek_char(lexer) == '_') {
        read_char(lexer);
    }
    
    size_t length = lexer->current_pos - start_pos;
    char* lexeme = (char*)malloc(length + 1);
    strncpy(lexeme, lexer->input[start_line-1] + start_pos, length);
    lexeme[length] = '\0';
    
    TokenType type = is_keyword(lexeme) ? TOKEN_KEYWORD : TOKEN_IDENTIFIER;
    return create_token(type, lexeme, start_line, start_column);
}

Token* read_symbols(Lexer* lexer) {
    char str[2];
    str[0] = read_char(lexer);
    str[1] = 0;
    TokenType type = TOKEN_OPERATOR;
    if (str[0] == '#') {
        type = TOKEN_PREPROCESSOR_DIRECTIVE;
    }
    else if (strchr("(){}[];,.", str[0])) {
        type = TOKEN_PUNCTUATOR;
    }
    else if (str[0] == '/') {
        char c = peek_char(lexer);
        if (c == '/' || c == '*') {
            skip_comment(lexer);
            return get_next_token(lexer);
        }
    }
    
    Token* token = create_token(type, str, lexer->line, lexer->column-1);
    return token;
}

Token* get_next_token(Lexer* lexer) {
    while (1) {
        char c = peek_char(lexer);
        
        if (c == '\0') {
            return create_token(TOKEN_EOF, "", lexer->line, lexer->column);
        }
        
        if (isspace(c)) {
            read_char(lexer);
            continue;
        }

        if (isalpha(c) || c == '_') {
            return read_identifier(lexer);
        }

        if (strchr("#+-*/=<>!&|^%(){}[];,.", c)) {
            return read_symbols(lexer);
        }

        return create_token(TOKEN_OPERATOR, "", lexer->line, lexer->column);
    }
}