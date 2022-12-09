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

            case SHAPE_DOT: assert(0 && "Single dot!");

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
        RETURN_ONE(SHAPE_ZERO)
        RETURN_ONE(SHAPE_ONE)
        RETURN_ONE(SHAPE_TWO)
        RETURN_ONE(SHAPE_THREE)
        RETURN_ONE(SHAPE_FOUR)
        RETURN_ONE(SHAPE_FIVE)
        RETURN_ONE(SHAPE_SIX)
        RETURN_ONE(SHAPE_SEVEN)
        RETURN_ONE(SHAPE_EIGHT)
        RETURN_ONE(SHAPE_NINE)
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
