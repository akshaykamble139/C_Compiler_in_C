#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_BUFFER 1024
#define SQUARE(x) ((x) * (x))
#define DEBUG_PRINT(x) printf("Debug: %s = %d\n", #x, x)

// Complex number structure
typedef struct {
    double real;
    double imag;
} Complex;

// Function prototypes
Complex add_complex(Complex a, Complex b);
void print_complex(Complex c);

// Global variables
int global_var = 42;
const char* global_string = "Global string";

// Enum definition
enum Colors {
    RED,
    GREEN,
    BLUE
};

// Union definition
union Data {
    int i;
    float f;
    char str[20];
};

int main(int argc, char *argv[]) {
    // Local variables
    int x = 10, y = 20;
    float pi = 3.14159f;
    char c = 'A';
    char str[] = "Hello, World!";
    
    // Pointer declaration and initialization
    int *ptr = &x;
    
    // Array declaration
    int arr[5] = {1, 2, 3, 4, 5};
    
    // Conditional statements
    if (x < y) {
        printf("x is less than y\n");
    } else if (x > y) {
        printf("x is greater than y\n");
    } else {
        printf("x is equal to y\n");
    }
    
    // Switch statement
    enum Colors color = GREEN;
    switch (color) {
        case RED:
            printf("Color is red\n");
            break;
        case GREEN:
            printf("Color is green\n");
            break;
        case BLUE:
            printf("Color is blue\n");
            break;
        default:
            printf("Unknown color\n");
    }
    
    // Loops
    for (int i = 0; i < 5; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");
    
    int j = 0;
    while (j < 3) {
        printf("While loop iteration %d\n", j++);
    }
    
    do {
        printf("Do-while loop iteration\n");
    } while (0);
    
    // Function call
    Complex a = {1.0, 2.0};
    Complex b = {3.0, 4.0};
    Complex result = add_complex(a, b);
    print_complex(result);
    
    // Macro usage
    int squared = SQUARE(5);
    DEBUG_PRINT(squared);
    
    // Bitwise operations
    int bitwise_and = 0b1010 & 0b1100;
    int bitwise_or = 0b1010 | 0b1100;
    int bitwise_xor = 0b1010 ^ 0b1100;
    int bitwise_not = ~0b1010;
    
    // Ternary operator
    int max = (x > y) ? x : y;
    
    // Memory allocation
    char *dynamic_str = (char *)malloc(MAX_BUFFER * sizeof(char));
    if (dynamic_str != NULL) {
        strcpy(dynamic_str, "Dynamically allocated string");
        printf("%s\n", dynamic_str);
        free(dynamic_str);
    }
    
    // Union usage
    union Data data;
    data.i = 10;
    printf("data.i: %d\n", data.i);
    data.f = 220.5f;
    printf("data.f: %.2f\n", data.f);
    strcpy(data.str, "C Programming");
    printf("data.str: %s\n", data.str);
    
    // Hexadecimal and octal literals
    int hex_num = 0xFF;
    int oct_num = 0755;
    
    // Character and string escape sequences
    printf("Newline\nTab\tBackslash\\\n");
    printf("Unicode character: \u03C0\n");
    
    return 0;
}

// Function definitions
Complex add_complex(Complex a, Complex b) {
    Complex result;
    result.real = a.real + b.real;
    result.imag = a.imag + b.imag;
    return result;
}

void print_complex(Complex c) {
    printf("%.2f + %.2fi\n", c.real, c.imag);
}