#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#if defined(_WIN32) || defined(_WIN64)
    #include <io.h>
#elif __linux__
    #include <unistd.h>
#else
    #error "Your system case is not defined!"
#endif

#include <ctype.h>
#include <string.h>
#include "libs/tree.hpp"
#include "libs/text.hpp"
#include "input-output.hpp"


#define check(condition, message, exitcode)                                                 \
do {                                                                                        \
    if (!(condition)) {                                                                     \
        printf("%s(%i) in %s\n%s\n", __FILE__, __LINE__, __PRETTY_FUNCTION__, message);     \
        return exitcode;                                                                    \
    }                                                                                       \
} while(0)


/**
 * \brief Prints node and it's children
 * \param [in]  node   Node to start from
 * \param [out] stream Output file
 * \param [in]  shift  Output text offset
*/
void write_node(Node *node, FILE *stream, int shift);


/**
 * \brief Removes all spaces from buffer
 * \param [out] buffer Buffer to remove from
*/
void clear_spaces(char *buffer);


/**
 * \brief Reads node from buffer
 * \param [in] buffer Char buffer to read from
*/
Node *read_node(char **buffer);




int write_tree(Tree *tree, const char *filepath) {
    check(tree, "Invalid pointer to tree!", 1);
    check(filepath, "Invalid pointer to filepath!", 2);

    FILE *output = fopen(filepath, "w");

    write_node(tree -> root, output, 0);

    fclose(output);
    return 0;
}


#define PRINT(...) fprintf(stream, __VA_ARGS__)

void write_node(Node *node, FILE *stream, int shift) {
    if (!node) return;

    PRINT("%*s{", shift, "");
    
    switch (node -> type) {
        case TYPE_OP:       PRINT("%i, %i",     TYPE_OP, node -> value.op);         break;
        case TYPE_NUM:      PRINT("%i, %.3lg",  TYPE_NUM, node -> value.dbl);       break;
        case TYPE_VAR:      PRINT("%i, %s",     TYPE_VAR, node -> value.var);       break;
        case TYPE_CALL:     PRINT("%i, %s",     TYPE_CALL, node -> value.var);      break;
        case TYPE_DEF:      PRINT("%i, %s",     TYPE_DEF, node -> value.var);       break;
        case TYPE_NVAR:     PRINT("%i, %s",     TYPE_NVAR, node -> value.var);      break;
        case TYPE_PAR:      PRINT("%i, %s",     TYPE_PAR, node -> value.var);       break;
        default:            PRINT("%i, 0",      node -> type);                      break;
    }

    if (node -> left) {
        PRINT(",\n");
        write_node(node -> left, stream, shift + 4);

        if (node -> right)
            PRINT("\n");
        else
            PRINT("\n%*s{ }\n", shift + 4, "");
    }
    
    if (node -> right) {
        if (!node -> left)
            PRINT(",\n%*s{ }\n", shift + 4, "");
        
        write_node(node -> right, stream, shift + 4);
        PRINT("\n");
    }

    if (node -> left || node -> right)
        PRINT("%*s}", shift, "");
    else
        PRINT("}");
}

#undef PRINT


void clear_spaces(char *buffer) {
    int read = 0, write = 0;

    for (; buffer[read] != '\0'; read++) {
        if (!isspace(buffer[read]))
            buffer[write++] = buffer[read];
    }

    buffer[write] = '\0';
}


int read_tree(Tree *tree, const char *filepath) {
    check(tree, "Invalid pointer to tree!", 1);
    check(filepath, "Invalid pointer to filepath!", 2);

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
