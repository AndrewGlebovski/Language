#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "libs/tree.hpp"
#include "image_parser.hpp"
#include "symbol_parser.hpp"
#include "grammar.hpp"




#define IS_TYPE(_type) ((*s) -> type == TYPE_##_type)
#define IS_OP(_op) ((*s) -> type == TYPE_OP && (*s) -> value.op == OP_##_op)


/// Increments node pointer
void next(Node **s);


Node *copy_node(const Node *origin) {
    Node *copy = (Node *) calloc(1, sizeof(Node));
    *copy = *origin;
    return copy;
}


Node *get_program(Node *str) {
    Node *s = str;
    
    Node *value = get_definition(&s);
    
    assert(s -> type == TYPE_ESC && "No TERMINATOR at the end of program!");

    return value;
}


Node *get_definition(Node **s) {
    Node *value = create_node(TYPE_DEF_SEQ, {0});

    switch ((*s) -> type) {
        case TYPE_NVAR: {
            next(s);

            value -> left = get_ident(s);

            value -> left -> type = TYPE_NVAR;

            assert(IS_OP(ASS) && "No assign in variable declaration!");
            next(s);

            value -> left -> right = get_expression(s);

            assert(value -> left -> right && "No expression to assign in variable declaration!");

            assert(IS_TYPE(SEQ) && "No ; after statement!");
            next(s);

            break;
        }
        case TYPE_DEF: {
            next(s);

            value -> left = get_ident(s);

            value -> left -> type = TYPE_DEF;

            assert(IS_TYPE(BRACKET) && (*s) -> value.op == 1 && "No opening bracket in function declaration!");
            next(s);

            if (!(IS_TYPE(BRACKET) && (*s) -> value.op == 0)) value -> left -> left = get_function_parameters(s);

            assert(IS_TYPE(BRACKET) && (*s) -> value.op == 0 && "No closing bracket in function declaration!");
            next(s);

            value -> left -> right = get_statement(s);

            break;
        }
        default: {
            free(value);
            return nullptr;
        }
    }

    value -> right = get_definition(s);

    return value;
}


Node *get_block_value(Node **s) {
    Node *value = get_statement(s);

    if (!(IS_TYPE(BLOCK) && (*s) -> value.op == 0))
        value -> right = get_block_value(s);

    return value;
}


Node *get_function_parameters(Node **s) {
    Node *value = get_ident(s);
    
    assert(value && "Wrong parameter name in function declaration!");

    if (IS_TYPE(CONT)) {
        next(s);

        value -> right = get_function_parameters(s);
    }

    value -> type = TYPE_PAR;

    return value;
}


Node *get_statement(Node **s) {
    Node *value = create_node(TYPE_SEQ, {0});

    switch((*s) -> type) {
        case TYPE_NVAR: {
            next(s);

            value -> left = get_ident(s);

            value -> left -> type = TYPE_NVAR;

            assert(IS_OP(ASS) && "No = after variable declaration!");
            next(s);

            value -> left -> right = get_expression(s);

            assert(value -> left -> right && "No expression after = in variable declaration!");

            assert(IS_TYPE(SEQ) && "No ; after statement!");
            next(s);

            break;
        }
        case TYPE_RET: {
            next(s);

            value -> left = create_node(TYPE_RET, {0}, get_expression(s));

            assert(value -> left -> left && "No expression after return!");

            assert(IS_TYPE(SEQ) && "No ; after statement!");
            next(s);

            break;
        }
        case TYPE_VAR: {
            Node *var = get_function(s);

            if (var -> type == TYPE_VAR) {
                assert(IS_OP(ASS) && "No assign operator after variable!");
                next(s);

                Node *exp = get_expression(s);

                assert(exp && "No expression after assign!");

                value -> left = create_node(TYPE_OP, {OP_ASS}, var, exp);
            }
            else {
                value -> left = var;
            }

            assert(IS_TYPE(SEQ) && "No ; after statement!");
            next(s);

            break;
        }
        case TYPE_BLOCK: {
            if ((*s) -> value.op == 1) {
                next(s);

                free(value);
                value = get_block_value(s);

                assert(IS_TYPE(BLOCK) && (*s) -> value.op == 0 && "No closing bracket in block!");
                next(s);
            }

            break;
        }
        case TYPE_IF: {
            value -> left = create_node(TYPE_IF, {0});
            next(s);

            assert(IS_TYPE(BRACKET) && (*s) -> value.op == 1 && "No opening bracket in if!");
            next(s);

            value -> left -> left = get_condition(s);

            assert(IS_TYPE(BRACKET) && (*s) -> value.op == 0 && "No closing bracket in if!");
            next(s);

            value -> left -> right = create_node(TYPE_BRANCH, {0}, get_statement(s));

            if (IS_TYPE(ELSE)) {
                next(s);

                value -> left -> right -> right = get_statement(s);
            }

            break;
        }
        case TYPE_WHILE: {
            value -> left = create_node(TYPE_WHILE, {0});
            next(s);

            assert(IS_TYPE(BRACKET) && (*s) -> value.op == 1 && "No opening bracket in if!");
            next(s);

            value -> left -> left = get_condition(s);

            assert(IS_TYPE(BRACKET) && (*s) -> value.op == 0 && "No closing bracket in if!");
            next(s);

            value -> left -> right = get_statement(s);

            break;
        }
        default: {
            free(value);
            value = nullptr;
        }
    }

    return value;
}


Node *get_ident(Node **s) {
    if (!IS_TYPE(VAR)) return nullptr;
    
    Node *value = copy_node(*s);

    next(s);

    return value;
}


Node *get_condition(Node **s) {
    Node *value = get_expression(s);

    if (IS_TYPE(OP)) {
        Node *op = copy_node(*s);
        next(s);

        op -> left = value, op -> right = get_expression(s);

        return op;
    }
    else {
        Node *op = create_node(TYPE_OP, {OP_NEQ}, value, create_node(TYPE_NUM, {0}));

        return op;
    }
}


Node *get_expression(Node **s) {
    Node *value = get_term(s);

    if (IS_OP(ADD) || IS_OP(SUB)) {
        Node *op = copy_node(*s); 
        next(s);

        op -> left = value, op -> right = get_expression(s);

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
        next(s);

        op -> left = value, op -> right = get_term(s);

        return op;
    }
    else {
        return value;
    }
}


Node *get_unary(Node **s) {
    if (IS_OP(SUB)) {
        Node *op = copy_node(*s);
        next(s);

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
        next(s);
        value = get_expression(s);

        assert(IS_TYPE(BRACKET) && (*s) -> value.op == 0 && "No closing bracket in expression!");
        next(s);

        return value;
    }
    else if ((value = get_function(s))) {
        return value;
    }
    else {
        return get_number(s);
    }
}


Node *get_function_arguments(Node **s) {
    Node *value = create_node(TYPE_ARG, {0}, get_expression(s), nullptr);
    
    assert(value -> left && "Wrong argument in function call!");

    if (IS_TYPE(CONT)) {
        next(s);

        value -> right = get_function_arguments(s);
    }

    return value;
}


Node *get_function(Node **s) {
    Node *value = get_ident(s);

    if (IS_TYPE(BRACKET) && (*s) -> value.op == 1) {
        next(s);

        value -> type = TYPE_CALL; 

        if (!(IS_TYPE(BRACKET) && (*s) -> value.op == 0)) value -> left = get_function_arguments(s);

        assert(IS_TYPE(BRACKET) && (*s) -> value.op == 0 && "No closing bracket in function call!");
        next(s);
    }

    return value;
}


Node *get_number(Node **s) {
    assert(IS_TYPE(NUM) && "No number found!");

    Node *value = copy_node(*s);

    next(s);

    return value;
}


void next(Node** s) {
    *s += 1;
}
