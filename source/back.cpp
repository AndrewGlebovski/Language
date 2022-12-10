#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <ctype.h>
#include "libs/tree.hpp"
#include "libs/text.hpp"




/**
 * \brief Removes all spaces from buffer
 * \param [out] buffer Buffer to remove from
*/
void clear_spaces(char *buffer);


/**
 * \brief Reads file into a tree
 * \param [out] tree     Not allocated tree
 * \param [in]  filepath Path to the file
 * \return Non zero value means error
*/
int read_tree(Tree *tree, const char *filepath);


/// Recursive read for node and its children
Node *read_node(char **buffer);




int main() {
    Tree tree = {};

    read_tree(&tree, "debug/output.txt");

    graphic_dump(&tree);

    tree_destructor(&tree);

    printf("Backend!");

    return 0;
}


void clear_spaces(char *buffer) {
    int read = 0, write = 0;

    for (; buffer[read] != '\0'; read++) {
        if (!isspace(buffer[read]))
            buffer[write++] = buffer[read];
    }

    buffer[write] = '\0';
}


int read_tree(Tree *tree, const char *filepath) {
    int input = open(filepath, O_RDONLY);

    char *buffer = nullptr;

    read_in_buffer(input, &buffer, get_file_size(input));

    replace_in_buffer(buffer, '\n', ' ');

    clear_spaces(buffer);

    char *buf = buffer;

    tree -> root = read_node(&buf);

    free(buffer);

    return 0;
}


Node *read_node(char **buffer) {
    if (strncmp(*buffer, "{}", 2) == 0) {
        *buffer += 2;
        return nullptr;
    }

    Node *node = create_node(0, {0});

    char *name = (char *) calloc(64, sizeof(char));

    int offset = 0;

    sscanf(*buffer, "{%i,%[^,}]%n", &node -> type, name, &offset);

    *buffer += offset;      // skip parsed symbols without ',' or '}'
    
    switch (node -> type) {
        case TYPE_OP:       node -> value.op = atoi(name); break;
        case TYPE_NUM:      node -> value.dbl = atof(name); break;
        case TYPE_VAR:      node -> value.var = name; break;
        case TYPE_CALL:     node -> value.var = name; break;
        case TYPE_DEF:      node -> value.var = name; break;
        case TYPE_NVAR:     node -> value.var = name; break;
        case TYPE_PAR:      node -> value.var = name; break;
        default:            free(name); break;
    }

    if (**buffer == ',') {
        *buffer += 1;       // skip ','

        node -> left = read_node(buffer);
        node -> right = read_node(buffer);
    }

    *buffer += 1;           // skip '}'

    return node;
}
