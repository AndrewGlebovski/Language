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

#include "frontend.hpp"




int main() {
    Image img = read_image("debug/code.png");
    
    Symbol *symbols = parse_image(&img);

    int symbols_size = img.width / (SYMBOL_SIZE + 1) * img.height / (SYMBOL_SIZE + 1);

    symbols_size = clear_empty(symbols, symbols_size);

    for(int i = 0; i < symbols_size; i++)
        print_symbol(symbols + i);

    free_image(&img);

    free(symbols);

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

    printf("x = %i, y = %i, color = ", symbol -> x, symbol -> y);
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


Symbol *parse_image(const Image *image) {
    assert(image && "Can't parse null image!");

    const int OFFSET = SYMBOL_SIZE + 1;

    assert(image -> width % OFFSET == 0 && image -> height % OFFSET == 0 && "Wrong image size!");

    Symbol *symbols = (Symbol *) calloc(image -> width / OFFSET * image -> height / OFFSET, sizeof(Symbol));

    for (int y = 0; y < image -> height / OFFSET; y++)
        for (int x = 0; x < image -> width / OFFSET; x++)
            symbols[x + y * image -> width / OFFSET] = get_image_symbol(image, x * OFFSET, y * OFFSET);
    
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
