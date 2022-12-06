#include <stdio.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wswitch-default"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wstack-usage="

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#pragma GCC diagnostic pop

#include "libs/tree.hpp"
#include "frontend.hpp"




int main() {
    Image img = read_image("debug/AssignExample.png");

    int size = 0;    

    Symbol *symbols = parse_image(&img, &size);

    free_image(&img);

    Node *tokens = parse_symbols(symbols, size, &size);

    free(symbols);
    
    print_tokens(tokens);

    free_tokens(tokens);

    printf("Frontend!");

    return 0;
}


int is_white(const Pixel *pixel) {
    assert(pixel && "Can't tell if null pixel is white or not!");

    return (pixel -> r == 255) && (pixel -> g == 255) && (pixel -> b == 255);
}


void print_pixel(const Pixel *pixel, const char format) {
    assert(pixel && "Null pixel can't be printed");

    if (format == 'x')
        printf("%02x%02x%02x%02x\n", pixel -> r, pixel -> g,  pixel -> b, pixel -> a);
    else if (format == 'i')
        printf("%03i %03i %03i %03i\n", pixel -> r, pixel -> g,  pixel -> b, pixel -> a);
}


Pixel *parse_pixels(int width, int height, const unsigned char *data) {
    assert(data && "Null data can't be parsed!");

    Pixel *pixels = (Pixel *) calloc(width * height, sizeof(Pixel));

    for(int i = 0; i < width * height; i++)
        pixels[i] = {data[i * 4], data[i * 4 + 1], data[i * 4 + 2], data[i * 4 + 3]};
    
    return pixels;
}


Image read_image(const char *filename) {
    assert(filename && "Image path is null!");

    Image img = {};
    int comp;

    unsigned char *data = stbi_load(filename, &img.width, &img.height, &comp, 0);

    assert(comp == 4 && "Can't work with less then 4 channels");

    img.pixels = parse_pixels(img.width, img.height, data);

    stbi_image_free(data);

    return img;
}


void free_image(Image *image) {
    assert(image && "Can't free null pointer!");
    assert(image -> pixels && "Can't free null pixel array!");

    free(image -> pixels);
    image -> pixels = nullptr;

    image -> width = -1;
    image -> height = -1;
}


Symbol get_image_symbol(const Image *image, int x, int y) {
    assert(image && "Can't get symbol from null image!");

    Symbol symbol = {{}, 0, x, y};
    unsigned int mask = 1;

    for (int _y = y; _y < y + SYMBOL_SIZE; _y++) {
        for (int _x = x; _x < x + SYMBOL_SIZE; _x++) {
            if (!is_white(image -> pixels + _x + _y * image -> width)) {
                symbol.shape |= mask;
                symbol.color = image -> pixels[_x + _y * image -> width];
            }

            mask <<= 1;
        }
    }

    return symbol;
}


void print_symbol(const Symbol *symbol) {
    assert(symbol && "Can't print null symbol!");

    printf("x = %i, y = %i, shape = %08X, color = ", symbol -> x, symbol -> y, symbol -> shape);
    print_pixel(&symbol -> color, 'x');

    unsigned int mask = 1;

    for (int y = 0; y < SYMBOL_SIZE; y++) {
        for (int x = 0; x < SYMBOL_SIZE; x++) {
            if (symbol -> shape & mask) putchar('*');
            else putchar(' ');

            mask <<= 1;
        }
        putchar('\n');
    }
}


Symbol *parse_image(const Image *image, int *symbols_size) {
    assert(image && "Can't parse null image!");

    const int OFFSET = SYMBOL_SIZE + 1;

    assert(image -> width % OFFSET == 0 && image -> height % OFFSET == 0 && "Wrong image size!");

    *symbols_size = image -> width / OFFSET * image -> height / OFFSET + 1;

    Symbol *symbols = (Symbol *) calloc(*symbols_size, sizeof(Symbol));

    for (int y = 0; y < image -> height / OFFSET; y++)
        for (int x = 0; x < image -> width / OFFSET; x++)
            symbols[x + y * image -> width / OFFSET] = get_image_symbol(image, x * OFFSET, y * OFFSET);
    
    symbols[*symbols_size - 1].shape = TERMINATOR;

    return symbols;
}


int clear_empty(Symbol *buffer, int buffer_size) {
    assert(buffer && "Can't clear empty buffer!");

    int read = 0, write = 0;

    for (; read < buffer_size; read++) {
        if (buffer[read].shape)
            buffer[write++] = buffer[read];
    }

    return write;
}


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
