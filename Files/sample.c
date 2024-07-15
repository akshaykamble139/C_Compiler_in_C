#include <stdio.h>
#include <stdlib.h>
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define SQUARE(x) ((x) * (x))

/* This is a multi-line comment
   with multiple lines */

// Single-line comment

typedef struct {
    int x;
    float y;
} Point;

enum Color {
    RED = 0xFF0000,
    GREEN = 0x00FF00,
    BLUE = 0x0000FF
};

int global_var = 42;

float calculate_distance(Point p1, Point p2) {
    return sqrt(SQUARE(p2.x - p1.x) + SQUARE(p2.y - p1.y));
}

int main(int argc, char *argv[]) {
    const char *message = "Hello, \"World\"!\n";
    printf("%s", message);

    // Different number representations
    int decimal = 12345;
    int octal = 0755;
    int hex = 0xABCDEF;
    float f1 = 3.14159f;
    double d1 = 2.71828e-10;

    // Bitwise operations
    int a = 0b1010 & 0b1100;
    int b = 0xFF | 0x0F;
    int c = ~0xA5;

    // Character literals
    char newline = '\n';
    char backslash = '\\';
    char single_quote = '\'';

    // Pointer arithmetic and dereference
    int *ptr = &global_var;
    *ptr += 5;

    // Array declaration and initialization
    int numbers[] = {1, 2, 3, 4, 5};
    
    // For loop with compound assignment
    for (int i = 0; i < 5; i++) {
        numbers[i] *= 2;
    }

    // Switch statement
    enum Color color = GREEN;
    switch (color) {
        case RED:
            printf("The color is red\n");
            break;
        case GREEN:
            printf("The color is green\n");
            break;
        case BLUE:
            printf("The color is blue\n");
            break;
        default:
            printf("Unknown color\n");
    }

    // Do-while loop
    int count = 0;
    do {
        count++;
    } while (count < 5);

    // Ternary operator
    int max = (argc > 1) ? atoi(argv[1]) : 10;

    // Function pointer
    float (*dist_func)(Point, Point) = &calculate_distance;

    Point p1 = {.x = 0, .y = 0};
    Point p2 = {.x = 3, .y = 4};
    printf("Distance: %f\n", dist_func(p1, p2));

    // Sizeof operator
    size_t size = sizeof(Point);

    // Comma operator
    int x = 1, y = 2, z = 3;

    // Preprocessor stringification
    #define PRINT_VAR(var) printf(#var " = %d\n", var)
    PRINT_VAR(x);

    return EXIT_SUCCESS;
}