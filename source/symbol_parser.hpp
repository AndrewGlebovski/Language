/// Bitmask for shape check
const unsigned int SHAPE_BTIMASK = 0x01FFFFFF;


#include "reserved_shapes.hpp"


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
