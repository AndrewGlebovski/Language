/**
 * \brief Prints tree to file in preorder
 * \param [in]  tree     To print
 * \param [out] filepath Output file
 * \return Non zero value means error
*/
int write_tree(Tree *tree, const char *filepath);


/**
 * \brief Reads file into a tree
 * \param [out] tree     Not allocated tree
 * \param [in]  filepath Path to the file
 * \return Non zero value means error
*/
int read_tree(Tree *tree, const char *filepath);
