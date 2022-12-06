#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "libs/tree.hpp"
#include "image_parser.hpp"
#include "symbol_parser.hpp"




#define CASE_TOKEN_TYPE(token_type) case token_type: token -> type = TYPE_##token_type; ptr++; break
#define CASE_TOKEN_OP(token_op) case token_op: token -> type = TYPE_OP; token -> value.op = OP_##token_op; ptr++; break

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
            CASE_TOKEN_TYPE(SEQ);

            case BRACKET_BEGIN: token -> type = TYPE_BLOCK; token -> value.dbl = 1.0; ptr++; break;
            case BRACKET_END:   token -> type = TYPE_BLOCK; token -> value.dbl = 0.0; ptr++; break;

            case BLOCK_BEGIN: token -> type = TYPE_BLOCK; token -> value.dbl = 1.0; ptr++; break;
            case BLOCK_END:   token -> type = TYPE_BLOCK; token -> value.dbl = 0.0; ptr++; break;
            
            CASE_TOKEN_OP(ADD);
            CASE_TOKEN_OP(SUB);
            CASE_TOKEN_OP(MUL);
            CASE_TOKEN_OP(DIV);

            CASE_TOKEN_OP(AND);
            CASE_TOKEN_OP(OR);
            CASE_TOKEN_OP(NOT);

            CASE_TOKEN_OP(ASS);

            case TERMINATOR: token -> type = TYPE_ESC; break; // Escape token

            default: {
                if (to_digit(ptr -> shape) != -1) {
                    token -> type = TYPE_NUM;

                    for(; to_digit(ptr -> shape) != -1; ptr++)
                        token -> value.dbl = token -> value.dbl * 10 + to_digit(ptr -> shape);
                        
                    if ((ptr -> shape & SHAPE_BTIMASK) == DOT) {
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
        case ZERO:      return 0;
        case ONE:       return 1;
        case TWO:       return 2;
        case THREE:     return 3;
        case FOUR:      return 4;
        case FIVE:      return 5;
        case SIX:       return 6;
        case SEVEN:     return 7;
        case EIGHT:     return 8;
        case NINE:      return 9;
        default:        return -1;
    }
}


#define RETURN_ONE(shape_name) case shape_name: return 1;

int is_reserved_shape(unsigned int shape) {
    switch (shape & SHAPE_BTIMASK) {
        RETURN_ONE(ZERO)
        RETURN_ONE(ONE)
        RETURN_ONE(TWO)
        RETURN_ONE(THREE)
        RETURN_ONE(FOUR)
        RETURN_ONE(FIVE)
        RETURN_ONE(SIX)
        RETURN_ONE(SEVEN)
        RETURN_ONE(EIGHT)
        RETURN_ONE(NINE)
        RETURN_ONE(SEQ)
        RETURN_ONE(BLOCK_BEGIN)
        RETURN_ONE(BLOCK_END)
        RETURN_ONE(BRACKET_BEGIN)
        RETURN_ONE(BRACKET_END)
        RETURN_ONE(IF)
        RETURN_ONE(WHILE)
        RETURN_ONE(NVAR)
        RETURN_ONE(ADD)
        RETURN_ONE(SUB)
        RETURN_ONE(MUL)
        RETURN_ONE(DIV)
        RETURN_ONE(AND)
        RETURN_ONE(OR)
        RETURN_ONE(NOT)
        RETURN_ONE(ASS)
        RETURN_ONE(DOT)
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
