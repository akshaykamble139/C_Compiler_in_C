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
    TOKEN_CHAR_LITERAL,
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
char peek_next_char(Lexer* lexer);
char read_char(Lexer* lexer);
void skip_comment(Lexer* lexer);
Token* create_token(TokenType type, char* lexeme, int line, int column);
Token* get_next_token(Lexer* lexer);
Token* read_identifier(Lexer* lexer);
int is_keyword(char* lexeme);

void free_resources(Lexer* lexer);
void report_error(Lexer* lexer, char* message);


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
    while (token_list[tokens_num] != NULL && token_list[tokens_num]->type != TOKEN_EOF) {
        printf("%d Token: type=%d, lexeme=\"%s\", line=%d, column=%d\n", tokens_num,
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

        tokens_num++;
    }

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
    if (lexer->line <= size && c == '\n') {
        lexer->on_a_new_line_char = 0;
        lexer->current_pos = 0;
        lexer->line++;
        lexer->column = 1;
        c = peek_char(lexer);
    }
    else {
        lexer->current_pos++;
        lexer->column++;
        lexer->on_a_new_line_char = c == '\n';
    }
    
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

void report_error(Lexer* lexer, char* message) {
    fprintf(stderr, "Lexical error at line %d, column %d: %s\n",
            lexer->line, lexer->column, message);
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

Token* read_operator(Lexer* lexer) {
    int start_pos = lexer->current_pos;
    int start_line = lexer->line;
    int start_column = lexer->column;
    
    char c1 = read_char(lexer);
    char c2 = peek_char(lexer);
    
    if (c2 != '\0') {
        char op[3] = {c1, c2, '\0'};
        if (strcmp(op, "==") == 0 || strcmp(op, "!=") == 0 ||
            strcmp(op, "<=") == 0 || strcmp(op, ">=") == 0 ||
            strcmp(op, "&&") == 0 || strcmp(op, "||") == 0 ||
            strcmp(op, "++") == 0 || strcmp(op, "--") == 0 ||
            strcmp(op, "+=") == 0 || strcmp(op, "-=") == 0 ||
            strcmp(op, "*=") == 0 || strcmp(op, "/=") == 0 ||
            strcmp(op, "%=") == 0 || strcmp(op, "&=") == 0 ||
            strcmp(op, "|=") == 0 || strcmp(op, "^=") == 0 ||
            strcmp(op, "<<") == 0 || strcmp(op, ">>") == 0) {
            read_char(lexer);  
            return create_token(TOKEN_OPERATOR, op, start_line, start_column);
        }
    }
    
    if (c2 != '\0' && peek_next_char(lexer) != '\0') {
        char op[4] = {c1, c2, peek_next_char(lexer), '\0'};
        if (strcmp(op, "<<=") == 0 || strcmp(op, ">>=") == 0) {
            read_char(lexer);  
            read_char(lexer);  
            return create_token(TOKEN_OPERATOR, op, start_line, start_column);
        }
    }
    
    char op[2] = {c1, '\0'};
    return create_token(TOKEN_OPERATOR, op, start_line, start_column);
}

Token* read_symbols(Lexer* lexer) {
    char str[2];
    str[0] = read_char(lexer);
    str[1] = 0;
    TokenType type = TOKEN_PUNCTUATOR;
    
    Token* token = create_token(type, str, lexer->line, lexer->column-1);
    return token;
}

char peek_next_char(Lexer* lexer) {
    if (lexer->current_pos + 1 >= strlen(lexer->input[lexer->line-1])) {
        return '\0';
    }
    return lexer->input[lexer->line-1][lexer->current_pos + 1];
}

Token* read_number(Lexer* lexer) {
    int start_pos = lexer->current_pos;
    int start_line = lexer->line;
    int start_column = lexer->column;
    TokenType type = TOKEN_INTEGER;
    int base = 10;

    if (peek_char(lexer) == '0' && (peek_next_char(lexer) == 'x' || peek_next_char(lexer) == 'X')) {
        read_char(lexer);  
        read_char(lexer); 
        base = 16;
    }
    else if (peek_char(lexer) == '0' && (peek_next_char(lexer) == 'b' || peek_next_char(lexer) == 'B')) {
        read_char(lexer);  
        read_char(lexer);  
        base = 2;
    }

    while (1) {
        char c = peek_char(lexer);
        if ((base == 10 && isdigit(c)) ||
            (base == 16 && isxdigit(c)) ||
            (base == 2 && (c == '0' || c == '1'))) {
            read_char(lexer);
        } else {
            break;
        }
    }

    if (base == 10 && peek_char(lexer) == '.') {
        type = TOKEN_FLOAT;
        read_char(lexer);  
        
        if (!isdigit(peek_char(lexer))) {
            report_error(lexer, "Expected digit after decimal point");
            return NULL;
        }
        while (isdigit(peek_char(lexer))) {
            read_char(lexer);
        }
    }

    if (base == 10 && (peek_char(lexer) == 'e' || peek_char(lexer) == 'E')) {
        type = TOKEN_FLOAT;
        read_char(lexer);  
        
        if (peek_char(lexer) == '+' || peek_char(lexer) == '-') {
            read_char(lexer);
        }
        
        if (!isdigit(peek_char(lexer))) {
            report_error(lexer, "Expected digit after exponent");
            return NULL;
        }
        while (isdigit(peek_char(lexer))) {
            read_char(lexer);
        }
    }

    char c = peek_char(lexer);
    if (c == 'u' || c == 'U' || c == 'l' || c == 'L' || c == 'f' || c == 'F') {
        read_char(lexer);
        if ((c == 'l' || c == 'L') && (peek_char(lexer) == 'l' || peek_char(lexer) == 'L')) {
            read_char(lexer);  
        }
    }

    size_t length = lexer->current_pos - start_pos;
    char* lexeme = (char*)malloc(length + 1);
    strncpy(lexeme, lexer->input[start_line-1] + start_pos, length);
    lexeme[length] = '\0';

    return create_token(type, lexeme, start_line, start_column);
}

Token* read_string(Lexer* lexer) {
    int start_pos = lexer->current_pos;
    int start_line = lexer->line;
    int start_column = lexer->column;
    
    read_char(lexer);
    
    char* buffer = malloc(1024);  
    size_t buffer_size = 1024;
    size_t length = 0;
    
    while (1) {
        char c = read_char(lexer);
        
        if (c == '"') {
            break;
        }
        
        if (c == '\0') {
            report_error(lexer, "Unterminated string literal");
            free(buffer);
            return NULL;
        }
        
        if (c == '\\') {
            c = read_char(lexer);
            switch (c) {
                case 'n': c = '\n'; break;
                case 't': c = '\t'; break;
                case 'r': c = '\r'; break;
                case '\\': c = '\\'; break;
                case '"': c = '"'; break;
                case '\'': c = '\''; break;
                case '0': c = '\0'; break;
                default:
                    report_error(lexer, "Unknown escape sequence");
                    free(buffer);
                    return NULL;
            }
        }
        
        if (length + 1 >= buffer_size) {
            buffer_size *= 2;
            buffer = realloc(buffer, buffer_size);
            if (!buffer) {
                report_error(lexer, "Memory allocation failed");
                return NULL;
            }
        }
        
        buffer[length++] = c;
    }
    
    buffer[length] = '\0';  
    
    Token* token = create_token(TOKEN_STRING, buffer, start_line, start_column);
    
    free(buffer);  
    
    return token;
}

Token* read_char_literal(Lexer* lexer) {
    int start_pos = lexer->current_pos;
    int start_line = lexer->line;
    int start_column = lexer->column;
    
    read_char(lexer);
    
    char value;
    
    if (peek_char(lexer) == '\\') {
        read_char(lexer); 
        char c = read_char(lexer);
        switch (c) {
            case 'n': value = '\n'; break;
            case 't': value = '\t'; break;
            case 'r': value = '\r'; break;
            case '0': value = '\0'; break;
            case '\\': value = '\\'; break;
            case '\'': value = '\''; break;
            case '"': value = '"'; break;
            case 'x': {
                char hex[3] = {0};
                hex[0] = read_char(lexer);
                hex[1] = read_char(lexer);
                value = (char)strtol(hex, NULL, 16);
                break;
            }
            default:
                report_error(lexer, "Unknown escape sequence in character literal");
                return NULL;
        }
    } else {
        value = read_char(lexer);
    }
    
    if (read_char(lexer) != '\'') {
        report_error(lexer, "Unterminated character literal");
        return NULL;
    }
    
    char lexeme[4] = {'\'', value, '\'', '\0'};
    return create_token(TOKEN_CHAR_LITERAL, lexeme, start_line, start_column);
}

Token* read_preprocessor_directive(Lexer* lexer) {
    int start_pos = lexer->current_pos;
    int start_line = lexer->line;
    int start_column = lexer->column;
    
    while (peek_char(lexer) != '\n' && peek_char(lexer) != '\0') {
        read_char(lexer);
    }
    
    size_t length = lexer->current_pos - start_pos;
    char* lexeme = (char*)malloc(length + 1);
    strncpy(lexeme, lexer->input[start_line-1] + start_pos, length);
    lexeme[length] = '\0';
    
    return create_token(TOKEN_PREPROCESSOR_DIRECTIVE, lexeme, start_line, start_column);
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

        if (c == '/') {
            if (peek_next_char(lexer) == '/' || peek_next_char(lexer) == '*') {
                read_char(lexer);  
                skip_comment(lexer);
                continue;  
            } else {
                return read_symbols(lexer);
            }
        }

        if (isalpha(c) || c == '_') {
            return read_identifier(lexer);
        }

        if (isdigit(c)) {
            return read_number(lexer);
        }

        if (c == '\'') {
            return read_char_literal(lexer);
        }

        if (c == '"') {
            return read_string(lexer);
        }

        if (c == '#') {
            return read_preprocessor_directive(lexer);
        }

        if (strchr("+-*/%=<>!&|^~?", c)) {
            Token* op_token = read_operator(lexer);
            if (op_token) {
                return op_token;
            }
        }

        if (strchr("(){}[]:;,.", c)) {
            return read_symbols(lexer);
        }

        return NULL;
    }
}