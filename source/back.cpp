#include <stdio.h>
#include "libs/tree.hpp"
#include "input-output.hpp"




int main() {
    Tree tree = {};

    read_tree(&tree, "debug/output.txt");

    graphic_dump(&tree);

    tree_destructor(&tree);

    printf("Backend!");

    return 0;
}
