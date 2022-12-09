#include <stdio.h>
#include <stdlib.h>
#include "libs/tree.hpp"
#include "image_parser.hpp"
#include "symbol_parser.hpp"
#include "grammar.hpp"




int main() {
    Image img = read_image("debug/FunctionExample.png");

    int size = 0;    

    Symbol *symbols = parse_image(&img, &size);

    free_image(&img);
    
    Node *tokens = parse_symbols(symbols, size, &size);

    // print_tokens(tokens);

    free(symbols);
    
    Tree tree = {get_program(tokens), 0};

    free(tokens); // Remember that char * type values in nodes are not free yet

    graphic_dump(&tree);

    tree_destructor(&tree);
    
    printf("Frontend!");

    return 0;
}
