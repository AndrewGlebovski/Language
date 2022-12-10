#include <stdio.h>
#include "libs/tree.hpp"
#include "input-output.hpp"
#include "program.hpp"




int main() {
    Tree tree = {};

    read_tree(&tree, "debug/output.txt");

    graphic_dump(&tree);

    print_program(&tree, "debug/source.txt");

    tree_destructor(&tree);

    printf("Backend!");

    return 0;
}
