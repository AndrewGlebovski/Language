#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libs/tree.hpp"
#include "dif.hpp"
#include "dsl.hpp"


///Copies origin parameters in destination and then destroys the origin
void copy_node(Node *origin, Node *destination);


#define PRINT(...) fprintf(file, __VA_ARGS__)


#define DEF_GEN(op, create_node, ...)               \
    case OP_##op:                                   \
        result = create_node;                       \
        break;


Node *diff(const Node *node, const char *dif_var) {
    Node *result = nullptr;

    switch(node -> type) {
        case NODE_TYPES::TYPE_NUM:
            result = create_num(0);
            break;
        case NODE_TYPES::TYPE_VAR:
            if (strcmp(node -> value.var, dif_var)) result = create_num(1);
            else result = create_num(0);
            break;
        case NODE_TYPES::TYPE_OP:
            switch (node -> value.op) {
                #include "gen.hpp"
                default: 
                    return nullptr;
            }
            break;
        default:
            return nullptr;
    }
    
    return result;
}

#undef DEF_GEN


#define TEMPLATE(num_child, origin_child, num)      \
if (IS_NUM(num_child, num)) {                       \
    FREE(node -> num_child);                        \
    copy_node(node -> origin_child, node);          \
}


#define CONST_CHECK()                                               \
if (IS_TYPE(node -> left, NUM) && IS_TYPE(node -> right, NUM)) {    \
    node -> value.dbl = calc_value(node, 1.0);                      \
    node -> type = TYPE_NUM;                                        \
    FREE(node -> left);                                             \
    FREE(node -> right);                                            \
}


void copy_node(Node *origin, Node *destination) {
    destination -> type = origin -> type;
    destination -> value = origin -> value;
    destination -> left = origin -> left;
    destination -> right = origin -> right;

    free(origin);
}


#define DEF_GEN(op, create_node, opti_node, ...)    \
case OP_##op:                                       \
    opti_node;                                      \
    break;


void optimize(Node *node) {
    if (node -> type != NODE_TYPES::TYPE_OP)
        return;

    if (node -> left) optimize(node -> left);
    if (node -> right) optimize(node -> right);

    switch (node -> value.op) {
        #include "gen.hpp"
        default:
            return;
    }
}

#undef DEF_GEN


#define DEF_GEN(op, create_node, opti_node, calc_node, ...)    \
    case OP_##op:                                              \
        return calc_node;


double calc_value(const Node *node, double x) {
    switch (node -> type) {
        case TYPE_NUM: return node -> value.dbl;
        case TYPE_VAR: return x;
        case TYPE_OP:
            switch (node -> value.op) {
                #include "gen.hpp"
                default: 
                    return 0.0;
            }
        default: return 0.0;
    }
}

#undef DEF_GEN
