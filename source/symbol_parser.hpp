/// Bitmask for shape check
const unsigned int SHAPE_BTIMASK = 0x01FFFFFF;


/// List of reserved shapes
typedef enum {
    SHAPE_ZERO            = 0x00ECD66E,
    SHAPE_ONE             = 0x00E210C4,
    SHAPE_TWO             = 0x01E2224C,
    SHAPE_THREE           = 0x00641906,
    SHAPE_FOUR            = 0x0084394A,
    SHAPE_FIVE            = 0x0064184E,
    SHAPE_SIX             = 0x00E5384E,
    SHAPE_SEVEN           = 0x0022221E,
    SHAPE_EIGHT           = 0x00C9324C,
    SHAPE_NINE            = 0x00E4394E,
    SHAPE_SEQ             = 0x00221004,
    SHAPE_CONT            = 0x00221000,
    SHAPE_BLOCK_BEGIN     = 0x00610846,
    SHAPE_BLOCK_END       = 0x00C4210C,
    SHAPE_BRACKET_BEGIN   = 0x00410844,
    SHAPE_BRACKET_END     = 0x00442104,
    SHAPE_IF              = 0x0052F49D,
    SHAPE_WHILE           = 0x01F8DE1F,
    SHAPE_NVAR            = 0x0027ABC8,
    SHAPE_DEF             = 0x01F8D63F,
    SHAPE_RET             = 0x00417C44,
    SHAPE_ADD             = 0x00023880,
    SHAPE_SUB             = 0x00003800,
    SHAPE_MUL             = 0x00001000,
    SHAPE_DIV             = 0x00111110,
    SHAPE_AND             = 0x0118A944,
    SHAPE_OR              = 0x00452A31,
    SHAPE_NOT             = 0x00401084,
    SHAPE_ASS             = 0x000701C0,
    SHAPE_DOT             = 0x00400000,
    SHAPE_EQ              = 0x00039380,
    SHAPE_NEQ             = 0x00038380,
    SHAPE_GRE             = 0x00364183,
    SHAPE_LES             = 0x018304D8,
    SHAPE_GEQ             = 0x0099110F,
    SHAPE_LEQ             = 0x0124905E,
    SHAPE_ELSE            = 0x01B4AD2B,
    SHAPE_DIF             = 0x00E53908
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


int draw_program(const Tree *tree, Symbol *symbols);
