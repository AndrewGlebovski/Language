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
        case TYPE_OP:       PRINT("OP, %s", op2str(node -> value.op)); break;
        case TYPE_NUM:      PRINT("NUM, %lg", node -> value.dbl); break;
        case TYPE_VAR:      PRINT("VAR, %s", node -> value.var); break;
        case TYPE_IF:       PRINT("IF"); break;
        case TYPE_WHILE:    PRINT("WHILE"); break;
        case TYPE_CALL:     PRINT("CALL, %s", node -> value.var); break;
        case TYPE_DEF:      PRINT("DEF, %s", node -> value.var); break;
        case TYPE_NVAR:     PRINT("NVAR, %s", node -> value.var); break;
        case TYPE_ARG:      PRINT("ARG"); break;
        case TYPE_PAR:      PRINT("PAR, %s", node -> value.var); break;
        case TYPE_SEQ:      PRINT("SEQ"); break;
        case TYPE_BLOCK:    PRINT("BLOCK"); break;
        case TYPE_RET:      PRINT("RET"); break;
        default:            PRINT("ERROR"); break;
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
