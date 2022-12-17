#include <stdio.h>
#include "libs/tree.hpp"
#include "dif.hpp"
#include "input-output.hpp"


int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Incorrect command line argument!\n");
        return 1;
    }

    Tree tree = {};

    read_tree(&tree, argv[1]);

    optimize(tree.root);

    write_tree(&tree, argv[1]);

    printf("Middlend!");

    return 0;
}
