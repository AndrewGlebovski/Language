#include <stdio.h>
#include "libs/tree.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wswitch-default"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wstack-usage="

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#pragma GCC diagnostic pop


const int SYMBOL_SIZE = 5;


/// Contains information about pixel color
typedef struct {
    unsigned char r = 0;        ///< Red channel
    unsigned char g = 0;        ///< Green channel
    unsigned char b = 0;        ///< Blue channel
    unsigned char a = 0;        ///< Alpha channel
} Pixel;


/// Contains information about image size and pixels
typedef struct {
    Pixel *pixels = nullptr;    ///< Array of Pixel from top left corner to bottom right
    int width = -1;             ///< Image width in pixels
    int height = -1;            ///< Image height in pixels
} Image;


/// Contains information about lexem or token
typedef struct {
    Pixel color = {};           ///< Token pixels color
    unsigned int shape = 0;     ///< Bitflag that shows what pixels aren't white
    int x = 0;                  ///< 'x' of the token left top corner
    int y = 0;                  ///< 'y' of the token left top corner
} Token;




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


Token get_image_token(const Image *image, int x, int y) {
    assert(image && "Can't get token from null image!");

    Token token = {{}, 0, x, y};
    unsigned int mask = 1;

    for (int _y = y; _y < y + SYMBOL_SIZE; _y++) {
        for (int _x = x; _x < x + SYMBOL_SIZE; _x++) {
            if (!is_white(image -> pixels + _x + _y * image -> width)) {
                token.shape |= mask;
                token.color = image -> pixels[_x + _y * image -> width];
            }

            mask <<= 1;
        }
    }

    return token;
}


void print_token(const Token *token) {
    assert(token && "Can't print null token!");

    printf("x = %i, y = %i, color = ", token -> x, token -> y);
    print_pixel(&token -> color, 'x');

    unsigned int mask = 1;

    for (int y = 0; y < SYMBOL_SIZE; y++) {
        for (int x = 0; x < SYMBOL_SIZE; x++) {
            if (token -> shape & mask) putchar('*');
            else putchar(' ');

            mask <<= 1;
        }
        putchar('\n');
    }
}


Token *parse_image(const Image *image) {
    assert(image && "Can't parse null image!");
    assert(image -> width % (SYMBOL_SIZE + 1) == 0 && image -> height % (SYMBOL_SIZE + 1) == 0 && "Wrong image size!");

    Token *tokens = (Token *) calloc(image -> width / (SYMBOL_SIZE + 1) * image -> height / (SYMBOL_SIZE + 1), sizeof(Token));

    for (int y = 0; y < image -> height; y += SYMBOL_SIZE + 1)
        for (int x = 0; x < image -> width; x += SYMBOL_SIZE + 1)
            tokens[x + y * image -> width / (SYMBOL_SIZE + 1)] = get_image_token(image, x, y);
    
    return tokens;
}




int main() {
    Image img = read_image("debug/code.png");
    
    Token *tokens = parse_image(&img);

    for(int i = 0; i < img.width / (SYMBOL_SIZE + 1) * img.height / (SYMBOL_SIZE + 1); i++)
        if (tokens[i].shape) print_token(tokens + i);

    free_image(&img);

    free(tokens);

    printf("Frontend!");

    return 0;
}
