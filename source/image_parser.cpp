#include <stdio.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-qual"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wswitch-default"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wstack-usage="
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#pragma GCC diagnostic pop

#include "image_parser.hpp"




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


int remove_empty_symbols(Symbol *buffer, int buffer_size) {
    assert(buffer && "Can't clear empty buffer!");

    int read = 0, write = 0;

    for (; read < buffer_size; read++) {
        if (buffer[read].shape)
            buffer[write++] = buffer[read];
    }

    return write;
}


Pixel *symbols_to_pixels(int width, int height, const Symbol *symbols) {
    Pixel *pixels = (Pixel *) calloc(width * height * 36, sizeof(Pixel));

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int mask = 1;

            for (int i = 0; i < SYMBOL_SIZE; i++) {
                for (int j = 0; j < SYMBOL_SIZE; j++) {
                    if (symbols[y * width + x].shape & mask) pixels[(y * 6 + i) * width * 6 + (x * 6 + j)] = symbols[y * width + x].color;
                    else pixels[(y * 6 + i) * width * 6 + (x * 6 + j)] = {255, 255, 255, 255};

                    mask <<= 1;
                }
            }

            for (int i = 0; i < SYMBOL_SIZE + 1; i++) {
                pixels[(y * 6 + i) * width * 6 + (x * 6 + 5)] = {161, 161, 161, 255};
                pixels[(y * 6 + 5) * width * 6 + (x * 6 + i)] = {161, 161, 161, 255};
            }

        }
    }

    return pixels;
}


void write_image(const char *filename, int width, int height, const Pixel *pixels) {
    unsigned char *data = (unsigned char *) calloc(width * height * 4, sizeof(unsigned char));

    const int COMP = 4; // Amount of channels in image

    for (int i = 0; i < width * height; i++) {
        data[COMP * i + 0] = pixels[i].r;
        data[COMP * i + 1] = pixels[i].g;
        data[COMP * i + 2] = pixels[i].b;
        data[COMP * i + 3] = pixels[i].a;
    }

    stbi_write_png(filename, width, height, COMP, data, width * COMP);

    free(data);
}
