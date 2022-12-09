#include <stdio.h>
#include <stdlib.h>
#include "libs/tree.hpp"
#include "image_parser.hpp"
#include "symbol_parser.hpp"
#include "grammar.hpp"




/**
 * \brief Prints tree to file in preorder
 * \param [in]  tree     To print
 * \param [out] filepath Output file
 * \return Non zero value means error
*/
int write_tree(Tree *tree, const char *filepath);


/**
 * \brief Prints node and it's children
 * \param [in]  node   Node to start from
 * \param [out] stream Output file
 * \param [in]  shift  Output text offset
*/
void write_node(Node *node, FILE *stream, int shift);




int main() {
    Image img = read_image("debug/FunctionExample.png");

    int size = 0;    

    Symbol *symbols = parse_image(&img, &size);

    free_image(&img);
    
    Node *tokens = parse_symbols(symbols, size, &size);

    // print_tokens(tokens);

    free(symbols);
    
    Tree tree = {get_program(tokens), 0};

    free(tokens); // Remember that char * type values in nodes are not free yet

    graphic_dump(&tree);

    write_tree(&tree, "debug/output.txt");

    tree_destructor(&tree);
    
    printf("Frontend!");

    return 0;
}


int write_tree(Tree *tree, const char *filepath) {
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
        case TYPE_IF:       PRINT("%i, 0",      TYPE_IF);                           break;
        case TYPE_WHILE:    PRINT("%i, 0",      TYPE_WHILE);                        break;
        case TYPE_CALL:     PRINT("%i, %s",     TYPE_CALL, node -> value.var);      break;
        case TYPE_DEF:      PRINT("%i, %s",     TYPE_DEF, node -> value.var);       break;
        case TYPE_NVAR:     PRINT("%i, %s",     TYPE_NVAR, node -> value.var);      break;
        case TYPE_ARG:      PRINT("%i, 0",      TYPE_ARG);                          break;
        case TYPE_PAR:      PRINT("%i, %s",     TYPE_PAR, node -> value.var);       break;
        case TYPE_SEQ:      PRINT("%i, 0",      TYPE_SEQ);                          break;
        case TYPE_BLOCK:    PRINT("%i, 0",      TYPE_BLOCK);                        break;
        case TYPE_RET:      PRINT("%i, 0",      TYPE_RET);                          break;
        default:            PRINT("%i, 0",      0);                                 break;
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
#undef PRINT
