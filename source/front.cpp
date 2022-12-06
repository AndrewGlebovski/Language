#include <stdio.h>
#include <stdlib.h>
#include "libs/tree.hpp"
#include "image_parser.hpp"
#include "symbol_parser.hpp"




int main() {
    Image img = read_image("debug/AssignExample.png");

    int size = 0;    

    Symbol *symbols = parse_image(&img, &size);

    free_image(&img);

    Node *tokens = parse_symbols(symbols, size, &size);

    free(symbols);
    
    print_tokens(tokens);

    free_tokens(tokens);

    printf("Frontend!");

    return 0;
}
