#include <stdio.h>
#include <stdlib.h>
#include "tree.hpp"


#define ASSERT(condition, message, error)                                    \
do {                                                                         \
    if (!(condition)) {                                                      \
        printf("%s(%i) in %s\n[%i] %s\n",                                    \
            __FILE__, __LINE__, __PRETTY_FUNCTION__, error, message);        \
        return error;                                                        \
    }                                                                        \
} while(0)


/**
 * \brief Free node and its children
 * \param [in] node Node pointer
*/
void free_node(Node *node);


/**
 * \brief Prints node and its children
 * \param [in]  node   Node to print
 * \param [out] stream Output stream
*/
void print_node(Node *node, FILE *stream);


/**
 * \brief Find node by its value
 * \param [in]  node  Search will start from this node
 * \param [in]  value Searching for this value
 * \return Actual pointer if node was found or nullptr
*/
Node *fast_search(Node *node, int value);


/**
 * \brief Find node by its value and store path to it
 * \param [in]  tree  Search will start from this node
 * \param [in]  value Searching for this value
 * \param [out] path  Contains path to node (path ends with nullptr)
 * \return Actual pointer if node was found or nullptr
*/
Node *full_search(Node *node, int value, Node *path[]);


/// Writes node and it's child connections
void write_record(FILE *file, Node *node);




int tree_constructor(Tree *tree, Node *root) {
    ASSERT(tree, "Constructor can't work with null pointer to tree!", INVALID_ARG);

    if (root) {
        tree -> root = root;
        tree -> size = 1;
    }

    return 0;
}


Node *create_node(int type, NodeValue value, Node *left, Node *right) {
    Node *node = (Node *) calloc(1, sizeof(Node));

    if (node) {
        node -> type = type;
        node -> value = value;
        node -> left = left;
        node -> right = right;
    }

    return node;
}


int tree_destructor(Tree *tree) {
    ASSERT(tree, "Constructor can't work with null pointer to tree!", INVALID_ARG);
    ASSERT(tree -> root, "Tree has null root!", INVALID_ARG);

    free_node(tree -> root);

    tree -> root = nullptr;
    tree -> size = 0;

    return 0;
}


void free_node(Node *node) {
    if (!node) return;

    if (node -> left) {
        free_node(node -> left);
        node -> left = nullptr;
    }

    if (node -> right) {
        free_node(node -> right);
        node -> right = nullptr;
    }

    switch (node -> type) {
        case TYPE_OP:       node -> value.op = 0; break;
        case TYPE_NUM:      node -> value.dbl = 0.0; break;
        case TYPE_VAR:      free(node -> value.var); break;
        case TYPE_CALL:     free(node -> value.var); break;
        case TYPE_DEF:      free(node -> value.var); break;
        case TYPE_NVAR:     free(node -> value.var); break;
        case TYPE_PAR:      free(node -> value.var); break;
        default:            node -> value.dbl = 0.0;
    }

    free(node);
}


void print_tree(Tree *tree, FILE *stream) {
    // ADD ASSERT HERE
    
    print_node(tree -> root, stream);

    fputc('\n', stream);
}


void print_node(Node *node, FILE *stream) {
    if (!node) return;

    fprintf(stream, "( ");

    if (node -> left) print_node(node -> left, stream);

    switch (node -> type) {
        case TYPE_OP:       fprintf(stream, "%s", op2str(node -> value.op)); break;
        case TYPE_NUM:      fprintf(stream, "%g", node -> value.dbl); break;
        case TYPE_VAR:      fprintf(stream, "%s", node -> value.var); break;
        case TYPE_IF:       fprintf(stream, "IF"); break;
        case TYPE_WHILE:    fprintf(stream, "WHILE"); break;
        case TYPE_CALL:     fprintf(stream, "FUNC CALL"); break;
        case TYPE_DEF:      fprintf(stream, "FUNC DEF"); break;
        case TYPE_NVAR:     fprintf(stream, "NEW VAR"); break;
        case TYPE_ARG:      fprintf(stream, "ARG"); break;
        case TYPE_PAR:      fprintf(stream, "PAR"); break;
        case TYPE_SEQ:      fprintf(stream, ";"); break;
        case TYPE_BLOCK:    fprintf(stream, "BLOCK"); break;
        case TYPE_RET:      fprintf(stream, "RETURN"); break;
        case TYPE_CONT:     fprintf(stream, ","); break;
        default:            fprintf(stream, "@");
    }

    if (node -> right) print_node(node -> right, stream);

    fprintf(stream, ") ");
}


const char *op2str(int op) {
    switch(op) {
        case OPERATORS::OP_ADD: return "+";
        case OPERATORS::OP_SUB: return "-";
        case OPERATORS::OP_MUL: return "*";
        case OPERATORS::OP_DIV: return "/";
        case OPERATORS::OP_AND: return "&";
        case OPERATORS::OP_OR:  return "&";
        case OPERATORS::OP_NOT: return "!";
        case OPERATORS::OP_ASS: return "=";
        case OPERATORS::OP_EQ:  return "==";
        case OPERATORS::OP_NEQ: return "!=";
        case OPERATORS::OP_GRE: return "\\>";
        case OPERATORS::OP_LES: return "\\<";
        case OPERATORS::OP_GEQ: return "\\>=";
        case OPERATORS::OP_LEQ: return "\\<=";
        default: return "#";
    }
}


int generate_file(Tree *tree, FILE *file) {
    fprintf(file, "digraph G {\n");

    fprintf(file, "    rankdir=HB\n");

    fprintf(file, "    node[shape=record, color=\"#355250\", fontsize=14, fontcolor=\"#355250\", style=\"filled\", fillcolor=\"#daf9f4\"];\n");
    fprintf(file, "    edge[color=\"#019aa2\"];\n");

    write_record(file, tree -> root);

    fprintf(file, "}\n");

    return 0;
}


void write_record(FILE *file, Node *node) {
    fprintf(file, "    \"e%p\"[label=\"{<index> %-p | {<type> ", node, node);

    switch (node -> type) {
        case TYPE_OP:       fprintf(file, "OP | <value> %s", op2str(node -> value.op)); break;
        case TYPE_NUM:      fprintf(file, "NUM | <value> %g", node -> value.dbl); break;
        case TYPE_VAR:      fprintf(file, "VAR | <value> %s", node -> value.var); break;
        case TYPE_IF:       fprintf(file, "IF"); break;
        case TYPE_WHILE:    fprintf(file, "WHILE"); break;
        case TYPE_CALL:     fprintf(file, "FUNC CALL | <value> %s", node -> value.var); break;
        case TYPE_DEF:      fprintf(file, "FUNC DEF | <value> %s", node -> value.var); break;
        case TYPE_NVAR:     fprintf(file, "NEW VAR | <value> %s", node -> value.var); break;
        case TYPE_ARG:      fprintf(file, "ARG"); break;
        case TYPE_PAR:      fprintf(file, "FUNC PAR | <value> %s", node -> value.var); break;
        case TYPE_SEQ:      fprintf(file, "SEQ"); break;
        case TYPE_BLOCK:    fprintf(file, "BLOCK"); break;
        case TYPE_RET:      fprintf(file, "RETURN"); break;
        case TYPE_CONT:     fprintf(file, ","); break;
        default:            fprintf(file, "@");
    }

    fprintf(file, "}}\"];\n");

    if (node -> left) {
        fprintf(file, "    \"e%p\" -> \"e%p\";\n", node, node -> left);
        write_record(file, node -> left);
    }

    if (node -> right) {
        fprintf(file, "    \"e%p\" -> \"e%p\";\n", node, node -> right);
        write_record(file, node -> right);
    }
}


int generate_image(const char *input, const char *output) {
    char cmd[20 + 2 * _MAX_PATH] = "";

    sprintf(cmd, "dot %s -o %s -Tpng", input, output);

    return system(cmd);
}


int show_image(const char *filepath) {
    char cmd[10 + _MAX_PATH] = "";

    sprintf(cmd, "start %s", filepath);

    system(cmd);

    return system("pause");
}


#define DUMP_DIRECTORY "debug"
#define DOT_FILENAME "dot-"
#define IMG_FILENAME "img-"


int graphic_dump(Tree *tree) {
    static int dump_index = 0;

    char dot[FILENAME_MAX] = "", img[FILENAME_MAX] = "";

    sprintf(dot, DUMP_DIRECTORY "/" DOT_FILENAME "%i.txt", dump_index);
    sprintf(img, DUMP_DIRECTORY "/" IMG_FILENAME "%i.png", dump_index);

    FILE *file = fopen(dot, "w");

    generate_file(tree, file);

    fclose(file);

    generate_image(dot, img);

    dump_index++;

    return 0;
}
