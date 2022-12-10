#include <stdio.h>
#include "libs/tree.hpp"
#include "input-output.hpp"
#include "program.hpp"




int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Incorrect command line argument!\n");
        return 1;
    }

    Tree tree = {};

    read_tree(&tree, argv[1]);

    graphic_dump(&tree);

    print_program(&tree, "source.txt");

    tree_destructor(&tree);

    printf("Backend!");

    return 0;
}
