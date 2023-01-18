#include <stdio.h>
#include <string.h>
#include "libs/tree.hpp"
#include "libs/parser.hpp"
#include "dif.hpp"
#include "input-output.hpp"




int main(int argc, char *argv[]) {
    char *ast_path = nullptr, *opti_ast_path = nullptr;

    Command command_list[] = {
        {
            "-i", "--input", 
            0, 
            &set_input_file, 
            &ast_path,
            "<filepath> Sets path to AST to optimize"
        },
        {
            "-o", "--output", 
            0, 
            &set_output_file, 
            &opti_ast_path,
            "<filepath> Sets path to save optimized AST (otherwise old one will be replaced)"
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

    optimize(tree.root);

    write_tree(&tree, (opti_ast_path)? opti_ast_path : ast_path);

    printf("Middlend!\n");

    return 0;
}
