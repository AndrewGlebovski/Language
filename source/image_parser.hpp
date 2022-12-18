/// Size of symbol in pixels
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


/// Contains information about symbol
typedef struct {
    Pixel color = {};           ///< Symbol pixels color
    unsigned int shape = 0;     ///< Bitflag that shows what pixels aren't white
    int x = 0;                  ///< 'x' of the Symbol left top corner
    int y = 0;                  ///< 'y' of the Symbol left top corner
} Symbol;


/// Terminator shape
const unsigned int TERMINATOR = 0x01FFFFFF;




/**
 * \brief Checks if pixel is white
 * \param [in] pixel To check
 * \return Non zero value means that pixel is white
*/
int is_white(const Pixel *pixel);


/**
 * \brief Prints pixel colors
 * \param [in] pixel  To print
 * \param [in] format 'i' - decimal, 'x' - heximal
*/
void print_pixel(const Pixel *pixel, const char format);


/**
 * \brief Parses pixels array from stbi data
 * \param [in] width  Image width
 * \param [in] height Image height
 * \param [in] data   Raw image data from stbi_load
 * \return Array of pixels
*/
Pixel *parse_pixels(int width, int height, const unsigned char *data);


/**
 * \brief Reads image size and its pixels colors
 * \brief [in] filename Path to file
 * \return New image
*/
Image read_image(const char *filename);


/**
 * \brief Image destructor
 * \param [in] image To destruct
*/
void free_image(Image *image);


/**
 * \brief Scans for symbol in image
 * \param [in] image To scan
 * \param [in] x     'x' of the symbol top left corner
 * \param [in] y     'y' of the symbol top left corner
 * \return New scanned symbol
*/
Symbol get_image_symbol(const Image *image, int x, int y);


/**
 * \brief Parses image to array of symbols
 * \param [in]  image        To parse
 * \param [out] symbols_size New symbols array size
 * \return Array of symbols
*/
Symbol *parse_image(const Image *image, int *symbols_size);


/**
 * \brief Prints all inforamtion about symbol
 * \param [in] symbol To print
*/
void print_symbol(const Symbol *symbol);


/**
 * \brief Removes all empty symbols from buffer
 * \param [out] buffer      Buffer to remove from
 * \param [in]  buffer_size Buffer actual size
 * \return New buffer size after cleaning
*/
int remove_empty_symbols(Symbol *buffer, int buffer_size);


/**
 * \brief Converts array of symbols to array of pixels
 * \param [in] width        Image width in symbols
 * \param [in] height       Image height in symbols
 * \param [in] symbols      Array of symbols to parse
 * \return Array of pixels with size width * height * 36
*/
Pixel *symbols_to_pixels(int width, int height, const Symbol *symbols);


/**
 * \brief Creates image from pixel array
 * \param [in] filename Path to image
 * \param [in] width    Image width in pixels
 * \param [in] height   Image height in pixels
 * \param [in] pixels   Array of pixels to work with
*/
void write_image(const char *filename, int width, int height, const Pixel *pixels);
