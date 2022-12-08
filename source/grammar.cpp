#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "libs/tree.hpp"
#include "image_parser.hpp"
#include "symbol_parser.hpp"
#include "grammar.hpp"




#define Call(left, right) create_node(TYPE_CALL, {0}, left, right)
#define Def(left, right) create_node(TYPE_DEF, {0}, left, right)
#define Arg(left, right) create_node(TYPE_ARG, {0}, left, right)
#define Par(left, right) create_node(TYPE_PAR, {0}, left, right)
#define Seq(left, right) create_node(TYPE_SEQ, {0}, left, right)

#define IS_TYPE(_type) ((*s) -> type == TYPE_##_type)
#define IS_OP(_op) ((*s) -> type == TYPE_OP && (*s) -> value.op == OP_##_op)


Node *copy_node(const Node *origin) {
    Node *copy = (Node *) calloc(1, sizeof(Node));
    *copy = *origin;
    return copy;
}


Node *get_program(Node *str) {
    Node *s = str;
    
    Node *value = get_statement(&s);
    
    assert(s -> type == TYPE_ESC && "No TERMINATOR at the end of program!");

    return value;
}


Node *get_block_value(Node **s) {
    Node *value = get_statement(s);

    if (!IS_TYPE(BLOCK))
        value -> right = get_block_value(s);

    return value;
}


Node *get_statement(Node **s) {
    Node *value = create_node(TYPE_SEQ, {0});

    if (IS_TYPE(NVAR)) {
        *s += 1;

        value -> left = get_ident(s);

        value -> left -> type = TYPE_NVAR;
    }
    else if (IS_TYPE(VAR)) {
        Node *var = get_ident(s);

        assert(IS_OP(ASS) && "No assign operator after variable!");
        *s += 1;

        Node *exp = get_expression(s);

        assert(exp && "No expression after assign!");

        value -> left = create_node(TYPE_OP, {OP_ASS}, var, exp);
    }
    else if (IS_TYPE(BLOCK) && (*s) -> value.op == 1) {
        value -> left = create_node(TYPE_BLOCK, {0});
        *s += 1;

        value -> left -> right = get_block_value(s);

        assert(IS_TYPE(BLOCK) && (*s) -> value.op == 0 && "No closing bracket in block!");
        *s += 1;
    }
    else {
        free(value);

        value = nullptr;
    }

    assert(IS_TYPE(SEQ) && "No ; after statement!");
    *s += 1;

    return value;
}


Node *get_ident(Node **s) {
    if (!IS_TYPE(VAR)) return nullptr;
    
    Node *value = copy_node(*s);

    *s += 1;

    return value;
}


Node *get_expression(Node **s) {
    Node *value = get_term(s);

    if (IS_OP(ADD) || IS_OP(SUB)) {
        Node *op = copy_node(*s); 

        *s += 1;
        Node *tmp = get_expression(s);

        op -> left = value, op -> right = tmp;

        return op;
    }
    else {
        return value;
    }
}


Node *get_term(Node **s) {
    Node *value = get_unary(s);

    if (IS_OP(MUL) || IS_OP(DIV)) {
        Node *op = copy_node(*s);

        *s += 1;
        Node *tmp = get_term(s);

        op -> left = value, op -> right = tmp;

        return op;
    }
    else {
        return value;
    }
}


Node *get_unary(Node **s) {
    if (IS_OP(SUB)) {
        Node *op = copy_node(*s);

        *s += 1;

        op -> left = create_node(TYPE_NUM, {0}), op -> right = get_factor(s);

        return op;
    }
    else {
        return get_factor(s);
    }
}


Node *get_factor(Node **s) {
    Node *value = {};

    if (IS_TYPE(BRACKET) && (*s) -> value.op == 1) {
        *s += 1;
        value = get_expression(s);

        assert(IS_TYPE(BRACKET) && (*s) -> value.op == 0 && "No closing bracket in expression!");

        *s += 1;

        return value;
    }
    else if ((value = get_ident(s))) {
        return value;
    }
    else {
        return get_number(s);
    }
}


Node *get_number(Node **s) {
    assert(IS_TYPE(NUM) && "No number found!");

    Node *value = copy_node(*s);

    *s += 1;

    return value;
}
