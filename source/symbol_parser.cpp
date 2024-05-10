#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "libs/tree.hpp"
#include "image_parser.hpp"
#include "symbol_parser.hpp"




#define CASE_TOKEN_TYPE(token_type) case SHAPE_##token_type: token -> type = TYPE_##token_type; ptr++; break
#define CASE_TOKEN_OP(token_op) case SHAPE_##token_op: token -> type = TYPE_OP; token -> value.op = OP_##token_op; ptr++; break

Node *parse_symbols(const Symbol *symbols, int symbols_size, int *tokens_size) {
    assert(symbols && "Can't parse null symbols!");
    assert(tokens_size && "Can't work with null tokens_size!");

    Node *tokens = (Node *) calloc(symbols_size, sizeof(Node));

    Node *token = tokens;
    const Symbol *ptr = symbols;

    for (; (ptr -> shape & SHAPE_BTIMASK) != TERMINATOR; token++) {
        while (!(ptr -> shape & SHAPE_BTIMASK)) ptr++;

        switch (ptr -> shape & SHAPE_BTIMASK) {
            CASE_TOKEN_TYPE(IF);
            CASE_TOKEN_TYPE(WHILE);
            CASE_TOKEN_TYPE(NVAR);
            CASE_TOKEN_TYPE(DEF);
            CASE_TOKEN_TYPE(SEQ);
            CASE_TOKEN_TYPE(CONT);
            CASE_TOKEN_TYPE(RET);
            CASE_TOKEN_TYPE(ELSE);

            case SHAPE_BRACKET_BEGIN: token -> type = TYPE_BRACKET; token -> value.op = 1; ptr++; break;
            case SHAPE_BRACKET_END:   token -> type = TYPE_BRACKET; token -> value.op = 0; ptr++; break;

            case SHAPE_BLOCK_BEGIN: token -> type = TYPE_BLOCK; token -> value.op = 1; ptr++; break;
            case SHAPE_BLOCK_END:   token -> type = TYPE_BLOCK; token -> value.op = 0; ptr++; break;
            
            CASE_TOKEN_OP(ADD);
            CASE_TOKEN_OP(SUB);
            CASE_TOKEN_OP(MUL);
            CASE_TOKEN_OP(DIV);

            CASE_TOKEN_OP(AND);
            CASE_TOKEN_OP(OR);
            CASE_TOKEN_OP(NOT);

            CASE_TOKEN_OP(EQ);
            CASE_TOKEN_OP(NEQ);
            CASE_TOKEN_OP(GRE);
            CASE_TOKEN_OP(LES);
            CASE_TOKEN_OP(GEQ);
            CASE_TOKEN_OP(LEQ);

            CASE_TOKEN_OP(ASS);

            CASE_TOKEN_OP(DIF);

            CASE_TOKEN_OP(REF);
            CASE_TOKEN_OP(LOC);

            case SHAPE_DOT: assert(0 && "Single dot!");

            case SHAPE_COM:
                ptr++;

                while (ptr -> shape != SHAPE_COM && ptr -> shape != TERMINATOR) ptr++;

                if (ptr -> shape == SHAPE_COM) ptr++;

                token--;  // Костыль так как token сдвигается на каждой итерации

                break;

            case TERMINATOR: token -> type = TYPE_ESC; break; // Escape token

            default: {
                if (to_digit(ptr -> shape) != -1) {
                    token -> type = TYPE_NUM;

                    for(; to_digit(ptr -> shape) != -1; ptr++)
                        token -> value.dbl = token -> value.dbl * 10 + to_digit(ptr -> shape);
                        
                    if ((ptr -> shape & SHAPE_BTIMASK) == SHAPE_DOT) {
                        int point = 1;

                        ptr++; // skip dot symbol

                        for(; to_digit(ptr -> shape) != -1; ptr++) {
                            token -> value.dbl = token -> value.dbl * 10 + to_digit(ptr -> shape);
                            point *= 10;
                        }
                        
                        assert(point > 1 && "No number after dot!");

                        token -> value.dbl /= (double) point;
                    }
                }
                else {
                    token -> type = TYPE_VAR;

                    token -> value.var = (char *) calloc(64, sizeof(char));

                    int offset = 0;

                    sprintf(token -> value.var, "VAR_%02X%02X%02X_%n", ptr -> color.r, ptr -> color.g, ptr -> color.b, &offset);

                    for (; ptr -> shape && !is_reserved_shape(ptr -> shape); ptr++) {
                        sprintf(token -> value.var + offset, "%08X", ptr -> shape);
                        offset += 8;

                        assert(offset < 56 && "Variable name is too large!");
                    }
                }

                break;
            }
        }
    }

    *tokens_size = (int) (token - tokens);

    return (Node *) realloc(tokens, *tokens_size * sizeof(Node));
}

#undef CASE_TOKEN_TYPE
#undef CASE_TOKEN_OP


int to_digit(unsigned int shape) {
    switch (shape & SHAPE_BTIMASK) {
        case SHAPE_ZERO:      return 0;
        case SHAPE_ONE:       return 1;
        case SHAPE_TWO:       return 2;
        case SHAPE_THREE:     return 3;
        case SHAPE_FOUR:      return 4;
        case SHAPE_FIVE:      return 5;
        case SHAPE_SIX:       return 6;
        case SHAPE_SEVEN:     return 7;
        case SHAPE_EIGHT:     return 8;
        case SHAPE_NINE:      return 9;
        default:              return -1;
    }
}


#define RETURN_ONE(shape_name) case shape_name: return 1;

int is_reserved_shape(unsigned int shape) {
    switch (shape & SHAPE_BTIMASK) {
        RETURN_ONE(SHAPE_SEQ)
        RETURN_ONE(SHAPE_CONT)
        RETURN_ONE(SHAPE_RET)
        RETURN_ONE(SHAPE_BLOCK_BEGIN)
        RETURN_ONE(SHAPE_BLOCK_END)
        RETURN_ONE(SHAPE_BRACKET_BEGIN)
        RETURN_ONE(SHAPE_BRACKET_END)
        RETURN_ONE(SHAPE_IF)
        RETURN_ONE(SHAPE_WHILE)
        RETURN_ONE(SHAPE_NVAR)
        RETURN_ONE(SHAPE_DEF)
        RETURN_ONE(SHAPE_ADD)
        RETURN_ONE(SHAPE_SUB)
        RETURN_ONE(SHAPE_MUL)
        RETURN_ONE(SHAPE_DIV)
        RETURN_ONE(SHAPE_AND)
        RETURN_ONE(SHAPE_OR)
        RETURN_ONE(SHAPE_NOT)
        RETURN_ONE(SHAPE_ASS)
        RETURN_ONE(SHAPE_DOT)
        RETURN_ONE(SHAPE_EQ)
        RETURN_ONE(SHAPE_NEQ)
        RETURN_ONE(SHAPE_GRE)
        RETURN_ONE(SHAPE_LES)
        RETURN_ONE(SHAPE_GEQ)
        RETURN_ONE(SHAPE_LEQ)
        RETURN_ONE(SHAPE_ELSE)
        RETURN_ONE(SHAPE_DIF)
        RETURN_ONE(SHAPE_COM)
        RETURN_ONE(TERMINATOR)
        default: return 0;
    }
}

#undef RETURN_ONE


void print_tokens(const Node *tokens) {
    for(; tokens -> type != TYPE_ESC; tokens++) {
        printf("Type: %i ", tokens -> type);

        switch (tokens -> type) {
            case TYPE_OP:  printf("%s", op2str(tokens -> value.op)); break;
            case TYPE_NUM: printf("%lg", tokens -> value.dbl); break;
            case TYPE_VAR: printf("%s", tokens -> value.var); break;
            case TYPE_BLOCK:  printf("%i", tokens -> value.op); break;
            case TYPE_BRACKET:  printf("%i", tokens -> value.op); break;
            default: break;
        }

        putchar('\n');
    }
}


void free_tokens(Node *tokens) {
    for(int i = 0; tokens[i].type != TYPE_ESC; i++)
        if (tokens[i].type == TYPE_VAR) free(tokens[i].value.var);
    
    free(tokens);
}


const Pixel GREEN_PIXEL = {34, 177, 77, 255};

void next(Symbol **s);

void draw_ident(const char *str, Symbol **s);

void draw_number(double num, Symbol **s);

void draw_skip(int shift, Symbol **s);

void draw_define_sequence(const Node *node, Symbol **s);

void draw_sequence(const Node *node, Symbol **s, int shift);

void draw_new_variable(const Node *node, Symbol **s);

void draw_new_function(const Node *node, Symbol **s, int shift);

void draw_expression(const Node *node, Symbol **s);

void draw_assign(const Node *node, Symbol **s);

void draw_if(const Node *node, Symbol **s, int shift);

void draw_while(const Node *node, Symbol **s, int shift);

void draw_function_call(const Node *node, Symbol **s);

void draw_return(const Node *node, Symbol **s);




void next(Symbol **s) {
    (*s) += 1;
}


void draw_ident(const char *str, Symbol **s) {
    int offset = 0;
    Pixel color = GREEN_PIXEL;

    sscanf(str, "VAR_%2hhx%2hhx%2hhx_%n", &color.r, &color.g, &color.b, &offset);

    for (const char *i = str + offset; *i; i += 8, next(s)) {
        unsigned int shape = 0;

        sscanf(i, "%8x", &shape);

        **s = {color, shape, 0, 0};
    }
}


void draw_number(double num, Symbol **s) {
    char str[25] = "";
    sprintf(str, "%lg", num);

    for (char *i = str; *i; i++, next(s)) {
        switch (*i) {
            case '0': **s = {GREEN_PIXEL, SHAPE_ZERO, 0, 0}; break;
            case '1': **s = {GREEN_PIXEL, SHAPE_ONE, 0, 0}; break;
            case '2': **s = {GREEN_PIXEL, SHAPE_TWO, 0, 0}; break;
            case '3': **s = {GREEN_PIXEL, SHAPE_THREE, 0, 0}; break;
            case '4': **s = {GREEN_PIXEL, SHAPE_FOUR, 0, 0}; break;
            case '5': **s = {GREEN_PIXEL, SHAPE_FIVE, 0, 0}; break;
            case '6': **s = {GREEN_PIXEL, SHAPE_SIX, 0, 0}; break;
            case '7': **s = {GREEN_PIXEL, SHAPE_SEVEN, 0, 0}; break;
            case '8': **s = {GREEN_PIXEL, SHAPE_EIGHT, 0, 0}; break;
            case '9': **s = {GREEN_PIXEL, SHAPE_NINE, 0, 0}; break;
            case '.': **s = {GREEN_PIXEL, SHAPE_DOT, 0, 0}; break;
            default: break;
        }
    }
}


void draw_skip(int shift, Symbol **s) {
    for (int i = 0; i < shift; i++, next(s))
        **s = {{255, 255, 255, 255}, 0, 0, 0};
}


int draw_program(const Tree *tree, Symbol *symbols) {
    Symbol *s = symbols;

    draw_define_sequence(tree -> root, &s);

    *s = {GREEN_PIXEL, TERMINATOR, 0, 0};

    return 0;
}


void draw_define_sequence(const Node *node, Symbol **s) {
    for (const Node *iter = node; iter; iter = iter -> right){
        switch (iter -> left -> type) {
            case TYPE_NVAR:     draw_new_variable(iter -> left, s); break;
            case TYPE_DEF:      draw_new_function(iter -> left, s, 0); break;
            default: return;
        }
    }
}


void draw_sequence(const Node *node, Symbol **s, int shift) {
    for (const Node *iter = node; iter; iter = iter -> right){
        draw_skip(shift, s);

        switch (iter -> left -> type) {
            case TYPE_NVAR:     draw_new_variable(iter -> left, s); break;
            case TYPE_OP:       draw_assign(iter -> left, s); break;
            case TYPE_IF:       draw_if(iter -> left, s, shift); break;
            case TYPE_WHILE:    draw_while(iter -> left, s, shift); break;
            case TYPE_RET:      draw_return(iter -> left, s); break;
            case TYPE_CALL:     draw_function_call(iter -> left, s); **s = {GREEN_PIXEL, SHAPE_SEQ, 0, 0}; next(s); break;
            default: printf("WTF!\n"); return;
        }
    }
}


void draw_new_variable(const Node *node, Symbol **s) {
    **s = {GREEN_PIXEL, SHAPE_NVAR, 0, 0};
    next(s);

    **s = {GREEN_PIXEL, SHAPE_ASS, 0, 0};
    next(s);

    draw_expression(node -> right, s);

    **s = {GREEN_PIXEL, SHAPE_SEQ, 0, 0};
    next(s);
}


void draw_new_function(const Node *node, Symbol **s, int shift) {
    **s = {GREEN_PIXEL, SHAPE_DEF, 0, 0};
    next(s);

    draw_ident(node -> value.var, s);

    **s = {GREEN_PIXEL, SHAPE_BRACKET_BEGIN, 0, 0};
    next(s);

    if (node -> left) {
        draw_ident(node -> left -> value.var, s);

        for (const Node *arg = node -> left -> right; arg; arg = arg -> right) {
            **s = {GREEN_PIXEL, SHAPE_CONT, 0, 0};
            next(s);

            draw_ident(arg -> value.var, s);
        }
    }

    **s = {GREEN_PIXEL, SHAPE_BRACKET_END, 0, 0};
    next(s);

    **s = {GREEN_PIXEL, SHAPE_BLOCK_BEGIN, 0, 0};
    next(s);

    draw_sequence(node -> right, s, shift + 1);

    **s = {GREEN_PIXEL, SHAPE_BLOCK_END, 0, 0};
    next(s);
}


void draw_expression(const Node *node, Symbol **s) {
    switch(node -> type) {
        case TYPE_NUM: {
            draw_number(node -> value.dbl, s);

            break;
        }
        case TYPE_VAR: {
            draw_ident(node -> value.var, s);

            break;
        }
        case TYPE_CALL: {
            draw_function_call(node, s);

            break;
        }
        case TYPE_OP: {
            draw_expression(node -> left, s);

            switch (node -> value.op) {
                case OP_ADD: **s = {GREEN_PIXEL, SHAPE_ADD, 0, 0}; break;
                case OP_SUB: **s = {GREEN_PIXEL, SHAPE_SUB, 0, 0}; break;
                case OP_MUL: **s = {GREEN_PIXEL, SHAPE_MUL, 0, 0}; break;
                case OP_DIV: **s = {GREEN_PIXEL, SHAPE_DIV, 0, 0}; break;

                case OP_EQ:  **s = {GREEN_PIXEL, SHAPE_EQ, 0, 0}; break;
                case OP_NEQ: **s = {GREEN_PIXEL, SHAPE_NEQ, 0, 0}; break;
                case OP_GRE: **s = {GREEN_PIXEL, SHAPE_GRE, 0, 0}; break;
                case OP_LES: **s = {GREEN_PIXEL, SHAPE_LES, 0, 0}; break;
                case OP_GEQ: **s = {GREEN_PIXEL, SHAPE_GEQ, 0, 0}; break;
                case OP_LEQ: **s = {GREEN_PIXEL, SHAPE_LEQ, 0, 0}; break;

                default: return;
            }

            next(s);

            draw_expression(node -> right, s);

            break;
        }
        default: return;
    }
}


void draw_assign(const Node *node, Symbol **s) {
    draw_ident(node -> left -> value.var, s);

    **s = {GREEN_PIXEL, SHAPE_ASS, 0, 0};
    next(s);

    draw_expression(node -> right, s);

    putchar('t');

    **s = {GREEN_PIXEL, SHAPE_SEQ, 0, 0};
    next(s);
}


void draw_if(const Node *node, Symbol **s, int shift) {
    **s = {GREEN_PIXEL, SHAPE_IF, 0, 0};
    next(s);

    **s = {GREEN_PIXEL, SHAPE_BRACKET_BEGIN, 0, 0};
    next(s);

    draw_expression(node -> left, s);

    **s = {GREEN_PIXEL, SHAPE_BRACKET_END, 0, 0};
    next(s);

    **s = {GREEN_PIXEL, SHAPE_BLOCK_BEGIN, 0, 0};
    next(s);

    draw_sequence(node -> right -> left, s, shift + 1);

    **s = {GREEN_PIXEL, SHAPE_BLOCK_END, 0, 0};
    next(s);

    if (node -> right -> right) {
        **s = {GREEN_PIXEL, SHAPE_ELSE, 0, 0};
        next(s);

        **s = {GREEN_PIXEL, SHAPE_BLOCK_BEGIN, 0, 0};
        next(s);

        draw_sequence(node -> right -> right, s, shift + 1);

        **s = {GREEN_PIXEL, SHAPE_BLOCK_END, 0, 0};
        next(s);
    }
}


void draw_while(const Node *node, Symbol **s, int shift) {
    **s = {GREEN_PIXEL, SHAPE_WHILE, 0, 0};
    next(s);

    **s = {GREEN_PIXEL, SHAPE_BRACKET_BEGIN, 0, 0};
    next(s);

    draw_expression(node -> left, s);

    **s = {GREEN_PIXEL, SHAPE_BRACKET_END, 0, 0};
    next(s);

    **s = {GREEN_PIXEL, SHAPE_BLOCK_BEGIN, 0, 0};
    next(s);

    draw_sequence(node -> right, s, shift + 1);

    **s = {GREEN_PIXEL, SHAPE_BLOCK_END, 0, 0};
    next(s);
}


void draw_function_call(const Node *node, Symbol **s) {
    draw_ident(node -> value.var, s);

    **s = {GREEN_PIXEL, SHAPE_BRACKET_BEGIN, 0, 0};
    next(s);

    if (node -> left) {
        draw_expression(node -> left, s);

        for (const Node *arg = node -> left -> right; arg; arg = arg -> right) {
            **s = {GREEN_PIXEL, SHAPE_CONT, 0, 0};
            next(s);

            draw_expression(arg, s);
        }
    }

    **s = {GREEN_PIXEL, SHAPE_BRACKET_END, 0, 0};
    next(s);
}


void draw_return(const Node *node, Symbol **s) {
    **s = {GREEN_PIXEL, SHAPE_RET, 0, 0};
    next(s);

    draw_expression(node -> left, s);

    **s = {GREEN_PIXEL, SHAPE_SEQ, 0, 0};
    next(s);
}
