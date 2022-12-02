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




int is_white(const Pixel *pixel) {
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


unsigned int get_image_token(Image *image, int x, int y) {
    unsigned int token = 0, mask = 1;

    for (int _y = y; _y < y + 5; _y++) {
        for (int _x = x; _x < x + 5; _x++) {
            if (!is_white(image -> pixels + _x + _y * image -> width)) 
                token |= mask;

            mask <<= 1;
        }
    }

    return token;
}


void print_token(unsigned int token) {
    unsigned int mask = 1;

    for (int y = 0; y < 5; y++) {
        for (int x = 0; x < 5; x++) {
            if (token & mask) putchar('*');
            else putchar(' ');

            mask <<= 1;
        }
        putchar('\n');
    }
}


void parse_image(Image *image) {
    assert(image -> width % 5 == 0 && image -> height % 5 == 0 && "Wrong image size!");

    for (int y = 0; y < image -> height; y += 5)
        for (int x = 0; x < image -> width; x += 5)
            print_token(get_image_token(image, x, y));
}




int main() {
    Image img = read_image("debug/assign.png");
    /*
    for (int i = 0; i < img.width * img.height; i++)
        print_pixel(img.pixels + i, 'i');
    */
    parse_image(&img);

    free_image(&img);

    printf("Frontend!");

    return 0;
}
