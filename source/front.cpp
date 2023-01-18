#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libs/tree.hpp"
#include "libs/parser.hpp"
#include "image_parser.hpp"
#include "symbol_parser.hpp"
#include "grammar.hpp"
#include "input-output.hpp"


void enable_graphic_dump(char *argv[], void *data);     ///< -gd parser



int main(int argc, char *argv[]) {
    char *image_path = nullptr, *ast_path = nullptr;
    int graphic_dump_on = 0;

    Command command_list[] = {
        {
            "-i", "--input", 
            0, 
            &set_input_file, 
            &image_path,
            "<filepath> Sets path to source file"
        },
        {
            "-o", "--output", 
            0, 
            &set_output_file, 
            &ast_path,
            "<filepath> Sets path to AST output"
        },
        {
            "-gd", "--graphic-dump", 
            0, 
            &enable_graphic_dump, 
            &graphic_dump_on,
            "Creates graphic representation of AST using GraphWiz"
        },
        {
            "-h", "--help", 
            0, 
            &show_help, 
            &command_list,
            "Prints all commands descriptions"
        },
    };

    parse_args(argc, argv, command_list, sizeof(command_list) / sizeof(Command));

    Image img = read_image(image_path);

    int size = 0;    

    Symbol *symbols = parse_image(&img, &size);

    free_image(&img);
    
    Node *tokens = parse_symbols(symbols, size, &size);

    free(symbols);
    
    Tree tree = {get_program(tokens), 0};

    free(tokens); // Remember that char * type values in nodes are not free yet

    if (graphic_dump_on) graphic_dump(&tree);

    write_tree(&tree, ast_path);

    tree_destructor(&tree);
    
    printf("Frontend!\n");

    return 0;
}




void enable_graphic_dump(char *argv[], void *data) {
    *((int *) data) = 1;
}
