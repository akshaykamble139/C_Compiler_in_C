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
} Lexer;

static const char* keywords[] = {
    "auto", "break", "case", "char", "const", "continue", "default", "do",
    "double", "else", "enum", "extern", "float", "for", "goto", "if",
    "int", "long", "register", "return", "short", "signed", "sizeof", "static",
    "struct", "switch", "typedef", "union", "unsigned", "void", "volatile", "while"
};

static const int num_keywords = sizeof(keywords) / sizeof(keywords[0]);

char* FILE_PATH = NULL;
char** lines = NULL;
size_t capacity = INITIAL_CAPACITY;
size_t size = 0;
Token** token_list = NULL;
size_t tokens_capacity = INITIAL_TOKEN_CAPACITY;
size_t tokens_num = 0;

char* get_C_File_name(int argc, char* argv[]);
int read_file_line_by_line();
int get_token_list(Lexer* lexer);
Lexer* init_lexer(char** program_lines);
char peek_char(Lexer* lexer);
char peek_next_char(Lexer* lexer);
char read_char(Lexer* lexer);
void skip_whitespace(Lexer* lexer);
void skip_comment(Lexer* lexer);
Token* create_token(TokenType type, char* lexeme, int line, int column);
Token* get_next_token(Lexer* lexer);
Token* read_identifier_or_keyword(Lexer* lexer);
Token* read_number(Lexer* lexer);
Token* read_string(Lexer* lexer);
Token* read_char_literal(Lexer* lexer);
Token* read_operator_or_punctuator(Lexer* lexer);
Token* read_preprocessor_directive(Lexer* lexer);
void free_resources(Lexer* lexer);
void report_error(Lexer* lexer, const char* message);

int main(int argc, char* argv[]) {
    FILE_PATH = get_C_File_name(argc, argv);
    
    if (FILE_PATH == NULL || read_file_line_by_line() != 0) {
        free_resources(NULL);
        return 1;
    }
    Lexer* lexer = init_lexer(lines);
    
    if (get_token_list(lexer) != 0) {
        free_resources(lexer);
        return 1;
    }

    free_resources(lexer);
    return 0;
}

void free_resources(Lexer* lexer) {
    free(FILE_PATH);
    for (size_t i = 0; i < size; i++) {
        free(lines[i]);
    }
    free(lines);

    for (size_t i = 0; i < tokens_num; i++) {
        free(token_list[i]->lexeme);
        free(token_list[i]);
    }
    free(token_list);

    free(lexer);
}

char* get_C_File_name(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return NULL;
    }

    const char* path = "./Files/";
    char* file_path = malloc(strlen(path) + strlen(argv[1]) + 1);
    if (file_path == NULL) {
        perror("Memory allocation failed");
        return NULL;
    }

    sprintf(file_path, "%s%s", path, argv[1]);
    return file_path;
}

int read_file_line_by_line() {
    FILE* file = fopen(FILE_PATH, "r");
    if (file == NULL) {
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

    char line[LINE_LENGTH];
    while (fgets(line, sizeof(line), file)) {
        if (size >= capacity) {
            capacity *= 2;
            char** new_lines = realloc(lines, capacity * sizeof(char*));
            if (new_lines == NULL) {
                perror("Error reallocating memory");
                fclose(file);
                return 1;
            }
            lines = new_lines;
        }

        line[strcspn(line, "\n")+1] = 0;

        lines[size] = strdup(line);
        if (lines[size] == NULL) {
            perror("Error allocating memory for line");
            fclose(file);
            return 1;
        }
        size++;
    }

    fclose(file);
    return 0;
}

int get_token_list(Lexer* lexer) {
    token_list = malloc(tokens_capacity * sizeof(Token*));
    if (token_list == NULL) {
        perror("Error allocating memory");
        return 1;
    }

    Token* token;
    while ((token = get_next_token(lexer)) != NULL) {
        if (tokens_num >= tokens_capacity) {
            tokens_capacity *= 2;
            Token** new_list = realloc(token_list, tokens_capacity * sizeof(Token*));
            if (new_list == NULL) {
                perror("Error reallocating memory");
                free(token);
                return 1;
            }
            token_list = new_list;
        }

        token_list[tokens_num++] = token;

        char* trimmed_lexeme = strdup(token->lexeme);
        size_t len = strlen(trimmed_lexeme);
        if (len > 0 && trimmed_lexeme[len-1] == '\n') {
            trimmed_lexeme[len-1] = '\0';
        }
        printf("%d Token: type=%d, lexeme=\"%s\", line=%d, column=%d\n", tokens_num,
               token->type, trimmed_lexeme, token->line, token->column);

        if (token->type == TOKEN_EOF) break;
    }

    return 0;
}

Lexer* init_lexer(char** program_lines) {
    Lexer* lexer = malloc(sizeof(Lexer));
    lexer->input = program_lines;
    lexer->line = 1;
    lexer->current_pos = 0;
    lexer->column = 1;
    return lexer;
}

char peek_char(Lexer* lexer) {
    if (lexer->line > size) return '\0';
    return lexer->input[lexer->line - 1][lexer->current_pos];
}

char peek_next_char(Lexer* lexer) {
    if (lexer->line > size) return '\0';
    return lexer->input[lexer->line - 1][lexer->current_pos + 1];
}

char read_char(Lexer* lexer) {
    char c = peek_char(lexer);
    if (lexer->line <= size && c == '\n') {
        lexer->line++;
        lexer->current_pos = 0;
        lexer->column = 1;
    } else {
        lexer->current_pos++;
        lexer->column++;
    }
    return c;
}

void skip_whitespace(Lexer* lexer) {
    while (isspace(peek_char(lexer))) {
        read_char(lexer);
    }
}

void skip_comment(Lexer* lexer) {
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
                report_error(lexer, "Unterminated multi-line comment");
                break;
            }
        }
    }
}

Token* create_token(TokenType type, char* lexeme, int line, int column) {
    Token* token = malloc(sizeof(Token));
    token->type = type;
    token->lexeme = strdup(lexeme);
    token->line = line;
    token->column = column;
    return token;
}

Token* get_next_token(Lexer* lexer) {
    skip_whitespace(lexer);

    char c = peek_char(lexer);
    if (c == '\0') return create_token(TOKEN_EOF, "", lexer->line, lexer->column);

    if (c == '/') {
        if (peek_next_char(lexer) == '/' || peek_next_char(lexer) == '*') {
            read_char(lexer);
            skip_comment(lexer);
            return get_next_token(lexer);
        }
    }

    if (isalpha(c) || c == '_') return read_identifier_or_keyword(lexer);
    if (isdigit(c)) return read_number(lexer);
    if (c == '"') return read_string(lexer);
    if (c == '\'') return read_char_literal(lexer);
    if (c == '#') return read_preprocessor_directive(lexer);

    return read_operator_or_punctuator(lexer);
}

Token* read_identifier_or_keyword(Lexer* lexer) {
    int start_pos = lexer->current_pos;
    int start_line = lexer->line;
    int start_column = lexer->column;
    
    while (isalnum(peek_char(lexer)) || peek_char(lexer) == '_') {
        read_char(lexer);
    }
    
    size_t length = lexer->current_pos - start_pos;
    char* lexeme = strndup(lexer->input[start_line-1] + start_pos, length);
    
    for (int i = 0; i < num_keywords; i++) {
        if (strcmp(lexeme, keywords[i]) == 0) {
            return create_token(TOKEN_KEYWORD, lexeme, start_line, start_column);
        }
    }
    
    return create_token(TOKEN_IDENTIFIER, lexeme, start_line, start_column);
}

Token* read_number(Lexer* lexer) {
    int start_pos = lexer->current_pos;
    int start_line = lexer->line;
    int start_column = lexer->column;
    TokenType type = TOKEN_INTEGER;
    int base = 10;

    if (peek_char(lexer) == '0') {
        read_char(lexer);
        if (peek_char(lexer) == 'x' || peek_char(lexer) == 'X') {
            read_char(lexer);
            base = 16;
        } else if (peek_char(lexer) == 'b' || peek_char(lexer) == 'B') {
            read_char(lexer);
            base = 2;
        } else {
            base = 8;
        }
    }

    while (1) {
        char c = peek_char(lexer);
        if ((base == 10 && isdigit(c)) ||
            (base == 16 && isxdigit(c)) ||
            (base == 8 && c >= '0' && c <= '7') ||
            (base == 2 && (c == '0' || c == '1'))) {
            read_char(lexer);
        } else {
            break;
        }
    }

    if (base == 10 && peek_char(lexer) == '.') {
        type = TOKEN_FLOAT;
        read_char(lexer);
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
    char* lexeme = strndup(lexer->input[start_line-1] + start_pos, length);

    return create_token(type, lexeme, start_line, start_column);
}

Token* read_string(Lexer* lexer) {
    int start_line = lexer->line;
    int start_column = lexer->column;
    
    read_char(lexer);  // Skip opening quote
    
    char* buffer = malloc(1024);
    size_t buffer_size = 1024;
    size_t length = 0;
    
    while (1) {
        char c = read_char(lexer);
        
        if (c == '"') break;
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
                case '\\': case '"': case '\'': break;
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
    int start_line = lexer->line;
    int start_column = lexer->column;
    
    read_char(lexer);  // Skip opening quote
    
    char value;
    if (peek_char(lexer) == '\\') {
        read_char(lexer);
        char c = read_char(lexer);
        switch (c) {
            case 'n': value = '\n'; break;
            case 't': value = '\t'; break;
            case 'r': value = '\r'; break;
            case '0': value = '\0'; break;
            case '\\': case '\'': case '"': value = c; break;
            case 'x': {
                char hex[3] = {read_char(lexer), read_char(lexer), 0};
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
    
    char lexeme[3] = {value, '\0'};
    return create_token(TOKEN_CHAR_LITERAL, lexeme, start_line, start_column);
}

Token* read_operator_or_punctuator(Lexer* lexer) {
    int start_line = lexer->line;
    int start_column = lexer->column;
    
    char c1 = read_char(lexer);
    char c2 = peek_char(lexer);
    
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
    
    if (strchr("+-*/%=<>!&|^~?", c1)) {
        op[1] = '\0';
        return create_token(TOKEN_OPERATOR, op, start_line, start_column);
    }
    
    if (strchr("(){}[]:;,.", c1)) {
        op[1] = '\0';
        return create_token(TOKEN_PUNCTUATOR, op, start_line, start_column);
    }
    
    char message[50];
    snprintf(message, sizeof(message), "Unexpected character: %c", c1);
    report_error(lexer, message);
    return NULL;
}

Token* read_preprocessor_directive(Lexer* lexer) {
    int start_line = lexer->line;
    int start_column = lexer->column;
    
    char* buffer = malloc(1024);
    size_t buffer_size = 1024;
    size_t length = 0;
    
    while (peek_char(lexer) != '\n' && peek_char(lexer) != '\0') {
        if (length + 1 >= buffer_size) {
            buffer_size *= 2;
            buffer = realloc(buffer, buffer_size);
            if (!buffer) {
                report_error(lexer, "Memory allocation failed");
                return NULL;
            }
        }
        buffer[length++] = read_char(lexer);
    }

    if (peek_char(lexer) == '\n') {
        read_char(lexer); 
    }
    
    buffer[length] = '\0';
    
    Token* token = create_token(TOKEN_PREPROCESSOR_DIRECTIVE, buffer, start_line, start_column);
    
    free(buffer);
    
    return token;
}

void report_error(Lexer* lexer, const char* message) {
    fprintf(stderr, "Error at line %d, column %d: %s\n", lexer->line, lexer->column, message);
}