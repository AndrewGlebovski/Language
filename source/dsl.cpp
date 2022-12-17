#include <stdio.h>
#include <math.h>
#include "libs/tree.hpp"
#include "dsl.hpp"




Node *create_num(double num) {
    NodeValue value = {0};
    value.dbl = num;
    return create_node(NODE_TYPES::TYPE_NUM, value, nullptr, nullptr);
}


Node *create_copy(const Node *node) {
    if (!node) return nullptr;
    
    return create_node(node -> type, node -> value, create_copy(node -> left), create_copy(node -> right));
}


int is_equal(double a, double b) {
    return fabs(a - b) < 1e-5;
}
