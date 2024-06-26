/**
 * \brief Prints program to assembler file
 * \param [in]  tree     Program tree to print
 * \param [out] filename Output file
*/
int print_program(const Tree *tree, const char *filename);


/**
 * \brief Calculates object hash sum
 * \param [in] ptr  Pointer to object
 * \param [in] size Object size in bytes
 * \return Hash value
*/
size_t gnu_hash(const void *ptr, size_t size);
