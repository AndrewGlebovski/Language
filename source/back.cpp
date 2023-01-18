#include <stdio.h>
#include "libs/parser.hpp"
#include "libs/tree.hpp"
#include "input-output.hpp"
#include "program.hpp"




int main(int argc, char *argv[]) {
    char *ast_path = nullptr, *asm_source_path = nullptr;

    Command command_list[] = {
        {
            "-i", "--input", 
            0, 
            &set_input_file, 
            &ast_path,
            "<filepath> Sets path to AST to compile"
        },
        {
            "-o", "--output", 
            0, 
            &set_output_file, 
            &asm_source_path,
            "<filepath> Sets path to save assembler source code"
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

    Tree tree = {};

    if (read_tree(&tree, ast_path)) return 1;

    // graphic_dump(&tree);

    print_program(&tree, asm_source_path);

    tree_destructor(&tree);

    printf("Backend!\n");

    return 0;
}
