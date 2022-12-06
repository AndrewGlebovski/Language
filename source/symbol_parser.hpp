/// Bitmask for shape check
const unsigned int SHAPE_BTIMASK = 0x01FFFFFF;


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
} RESERVED_SHAPES;


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
