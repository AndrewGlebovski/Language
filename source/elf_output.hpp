/**
 * \file
 * \brief Header for creating elf64 module
*/


const size_t START_ADDRESS = 0x400000;
const size_t ALIGN = 0x1000;


/// Holds information about global variables
typedef struct {
    size_t offset       = 0;
    double init_value   = 0.0;
} global_t;


/**
 * \brief Creates elf64 executable with the given Intermediate Representation
 * \param [in] elf_filename Path to new elf file
 * \param [in] ir           Intermediate representation
 * \param [in] globals      Global variables
 * \param [in] globals_size Size of the globals buffer
 * \return Non zero value means error
*/
int create_elf(const char *elf_filename, const IR *ir, const global_t *globals, size_t globals_size);
