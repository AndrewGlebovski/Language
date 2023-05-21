/**
 * \brief Creates elf64 executable with the given Intermediate Representation
 * \param [in] elf_filename Path to new elf file
 * \param [in] ir           Intermediate representation
 * \return Non zero value means error
*/
int create_elf(const char *elf_filename, const IR *ir);
