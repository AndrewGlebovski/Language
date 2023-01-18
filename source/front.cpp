#include <stdio.h>
#include <stdlib.h>
#include "libs/tree.hpp"
#include "image_parser.hpp"
#include "symbol_parser.hpp"
#include "grammar.hpp"
#include "input-output.hpp"




int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Incorrect command line argument!\n");
        return 1;
    }

    Image img = read_image(argv[1]);

    int size = 0;    

    Symbol *symbols = parse_image(&img, &size);

    free_image(&img);
    
    Node *tokens = parse_symbols(symbols, size, &size);

    // print_tokens(tokens);

    free(symbols);
    
    Tree tree = {get_program(tokens), 0};

    free(tokens); // Remember that char * type values in nodes are not free yet

    graphic_dump(&tree);

    write_tree(&tree, "output.txt");

    tree_destructor(&tree);
    
    printf("Frontend!\n");

    return 0;
}
