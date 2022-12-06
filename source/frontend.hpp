/// Size of symbol in pixels
const int SYMBOL_SIZE = 5;


/// Bitmask for shape check
const unsigned int SHAPE_BTIMASK = 0x01FFFFFF;


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


/// List of reserved shapes
typedef enum {
    ZERO            = 0x00ECD66E,
    ONE             = 0x00E210C4,
    TWO             = 0x01E2224C,
    THREE           = 0x00641906,
    FOUR            = 0x0084394A,
    FIVE            = 0x0064184E,
    SIX             = 0x00E5184E,
    SEVEN           = 0x0022221E,
    EIGHT           = 0x00C9324C,
    NINE            = 0x00E4394E,
    SEQ             = 0x00221004,
    BLOCK_BEGIN     = 0x00610846,
    BLOCK_END       = 0x00C4210C,
    BRACKET_BEGIN   = 0x00410844,
    BRACKET_END     = 0x00442104,
    IF              = 0x0052F49D,
    WHILE           = 0x00AAD6B5,
    NVAR            = 0x011CD671,
    ADD             = 0x00023880,
    SUB             = 0x00003800,
    MUL             = 0x00001000,
    DIV             = 0x00111110,
    AND             = 0x0118A944,
    OR              = 0x00452A31,
    NOT             = 0x00401084,
    ASS             = 0x000701C0,
    DOT             = 0x00400000,
    TERMINATOR      = 0x01FFFFFF,
} RESERVED_SHAPES;


/**
 * \brief Removes all empty symbols from buffer
 * \param [out] buffer      Buffer to remove from
 * \param [in]  buffer_size Buffer actual size
 * \return New buffer size after cleaning
*/
int clear_empty(Symbol *buffer, int buffer_size);


/**
 * \brief Prints all inforamtion about symbol
 * \param [in] symbol To print
*/
void print_symbol(const Symbol *symbol);


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
 * \brief Converts shape to digit
 * \param [in] shape To convert
 * \return Digit or -1 if fails
*/
int to_digit(unsigned int shape);


/**
 * \brief Checks if shape is in list of reserved shapes
 * \param [in] shape To check
 * \return Non zero value means value is reserved
*/
int is_reserved_shape(unsigned int shape);


/**
 * \brief Parses symbols to lexems
 * \param [in] symbols      To parse
 * \param [in] symbols_size Symbols array size
 * \param [in] tokens_size  Size of token array
 * \return Array of lexems
*/
Node *parse_symbols(const Symbol *symbols, int symbols_size, int *tokens_size);


/**
 * \brief Prints tokens type and value
 * \param [in] tokens To print
*/
void print_tokens(const Node *tokens);


/**
 * \brief Free tokens array and char * type values
 * \param [in] tokens To free
*/
void free_tokens(Node *tokens);
