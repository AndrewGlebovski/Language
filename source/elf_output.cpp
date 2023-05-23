#include <assert.h>
#include <elf.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <memory.h>
#include <sys/stat.h>
#include "inter.hpp"
#include "elf_output.hpp"
#include "my_stdlib.hpp"


/// Initial size of the bytecode buffer
const size_t BYTECODE_INIT_SIZE = ALIGN;


/**
 * \brief Writes elf to file
 * \param [in]  ir              Intermediate representation
 * \param [in]  globals         Array of global variables
 * \param [in]  globals_size    Amount of globals in program
 * \param [out] file            File desciptor
*/
void write_elf64(const IR *ir, const global_t *globals, size_t globals_size, int file);


/**
 * \brief Writes elf header to buffer
 * \param [out] buffer  Writes Elf64 header to buffer
 * \param [in]  phnum   Amount of program headers
*/
size_t create_elf64_header(uint8_t *buffer, Elf64_Half phnum);


/**
 * \brief Writes elf program table to buffer
 * \param [out] buffer          Writes Elf64 header to buffer
 * \param [in]  ir              Intermediate representation
 * \param [in]  globals_size    Amount of globals in program
 * \return Program table size
*/
size_t create_elf64_progtable(uint8_t *buffer, const IR *ir, size_t globals_size);


/**
 * \brief Writes elf program header
 * \param [out] buffer      Writes Elf64 header to buffer
 * \param [in]  offset      Segment offset in file
 * \param [in]  file_size   Segment size in file
 * \param [in]  flags       Segment flags
 * \return Program header size
*/
size_t create_elf64_progheader(uint8_t *buffer, Elf64_Off offset, Elf64_Xword file_size, Elf64_Word flags);


/**
 * \brief Writes binary file content to the output file
 * \param [in]  filename    Path to binary file
 * \param [out] output      Output file descriptor
 * \return Amount of bytes written
*/
size_t include_binary(const char *filename, uint8_t *buffer);


/**
 * \brief Sets bytes of the buffer to zero
 * \param [out] buffer      Buffer
 * \param [in]  buffer_size Buffer size
*/
void clear_buffer(uint8_t *buffer, size_t buffer_size);


/**
 * \brief Returns new size greater or equal to size and aligned to ALIGN constant
 * \param [in] size Size that needs to be aligned
 * \return Aligned size
*/
size_t get_aligned_size(size_t size);




int create_elf(const char *elf_filename, const IR *ir, const global_t *globals, size_t globals_size) {
    assert(elf_filename && "Null pointer!\n");
    assert(ir && "Null pointer!\n");

    mode_t old_umask = umask(0);

    int file = open(elf_filename, O_RDWR | O_CREAT | O_TRUNC, 00777);
    assert(file != -1 && "Couldn't open or creat such file!\n");

    write_elf64(ir, globals, globals_size, file);

    close(file);

    umask(old_umask);
    
    return 0;
}


#define BYTECODE_RESIZE(new_size, old_size)                                 \
do {                                                                        \
    bytecode = (uint8_t *) realloc(bytecode, new_size);                     \
    assert(bytecode && "Couldn't reallocate buffer for bytecode!\n");       \
    clear_buffer(bytecode + old_size, new_size - old_size);                 \
    bytecode_size = new_size;                                               \
} while (0)


void write_elf64(const IR *ir, const global_t *globals, size_t globals_size, int file) {
    assert(ir && "Null pointer!\n");
    assert(file != -1 && "File descriptor is -1!\n");

    uint8_t *bytecode = (uint8_t *) calloc(BYTECODE_INIT_SIZE, sizeof(uint8_t));
    assert(bytecode && "Couldn't allocate buffer for bytecode!\n");

    size_t seg_size = 0, aligned_size = 0, bytecode_size = BYTECODE_INIT_SIZE;

    seg_size += create_elf64_header(bytecode, (globals_size) ? 3 : 2);
    seg_size += create_elf64_progtable(bytecode + seg_size, ir, globals_size);
    assert(write(file, bytecode, bytecode_size) != -1 && "Write failed!\n");

    clear_buffer(bytecode, seg_size);

    seg_size = ir -> ip;
    aligned_size = get_aligned_size(seg_size);

    assert(aligned_size == bytecode_size && "Code segment couldn't shrink expend!\n");

    if (aligned_size > bytecode_size) BYTECODE_RESIZE(aligned_size, bytecode_size);

    assert(include_binary(STDLIB_FILE, bytecode) == STDLIB_SIZE && "Wrong amount of bytes written!\n");
    assert(!IR_write(ir, bytecode + STDLIB_SIZE) && "IR write failed!\n");

    if (!globals_size) {
        assert(write(file, bytecode, seg_size) != -1 && "Write failed!\n");
        return;
    }

    assert(write(file, bytecode, aligned_size) != -1 && "Write failed!\n");

    clear_buffer(bytecode, seg_size);

    seg_size = globals_size * 8;
    aligned_size = get_aligned_size(seg_size);

    if (aligned_size > bytecode_size) BYTECODE_RESIZE(aligned_size, bytecode_size);

    for (size_t i = 0; i < globals_size; i++) *((long *)bytecode + i) = (long)(globals[i].init_value * 1000.0);

    assert(write(file, bytecode, seg_size) != -1 && "Write failed!\n");

    free(bytecode);
}


size_t create_elf64_header(uint8_t *buffer, Elf64_Half phnum) {
    assert(buffer && "Null pointer!\n");

    Elf64_Ehdr *elf64 = (Elf64_Ehdr *) buffer;

    elf64 -> e_ident[EI_MAG0]       = ELFMAG0;
    elf64 -> e_ident[EI_MAG1]       = ELFMAG1;
    elf64 -> e_ident[EI_MAG2]       = ELFMAG2;
    elf64 -> e_ident[EI_MAG3]       = ELFMAG3;
    elf64 -> e_ident[EI_CLASS]      = ELFCLASS64;
    elf64 -> e_ident[EI_DATA]       = ELFDATA2LSB;
    elf64 -> e_ident[EI_VERSION]    = EV_CURRENT;
    elf64 -> e_ident[EI_OSABI]      = ELFOSABI_SYSV;
    elf64 -> e_ident[EI_ABIVERSION] = 0;
    elf64 -> e_ident[EI_PAD]        = 0;
    elf64 -> e_ident[EI_NIDENT]     = sizeof(elf64 -> e_ident);

    elf64 -> e_type                 = ET_EXEC;
    elf64 -> e_machine              = EM_X86_64;
    elf64 -> e_entry                = START_ADDRESS + ALIGN + STDLIB_SIZE;
    elf64 -> e_phoff                = sizeof(Elf64_Ehdr);
    elf64 -> e_shoff                = 0;
    elf64 -> e_version              = EV_CURRENT;
    elf64 -> e_flags                = 0;
    elf64 -> e_ehsize               = sizeof(Elf64_Ehdr);
    elf64 -> e_phentsize            = sizeof(Elf64_Phdr);
    elf64 -> e_phnum                = phnum;
    elf64 -> e_shentsize            = 0;
    elf64 -> e_shnum                = 0;
    elf64 -> e_shstrndx             = SHN_UNDEF;

    return sizeof(Elf64_Ehdr);
}


size_t create_elf64_progtable(uint8_t *buffer, const IR *ir, size_t globals_size) {
    assert(buffer && "Null pointer!\n");
    assert(ir && "Null pointer!\n");

    size_t size = 0, ph_count = (globals_size) ? 3 : 2;

    size += create_elf64_progheader(buffer, 0, sizeof(Elf64_Ehdr) + sizeof(Elf64_Phdr) * ph_count, PF_R);
    size += create_elf64_progheader(buffer + size, ALIGN, ir -> ip, PF_R | PF_X);

    if (!globals_size) return size;

    size += create_elf64_progheader(buffer + size, ALIGN + get_aligned_size(ir -> ip), globals_size * 8U, PF_R | PF_W);
    return size;
}


size_t create_elf64_progheader(uint8_t *buffer, Elf64_Off offset, Elf64_Xword file_size, Elf64_Word flags) {
    assert(buffer && "Null pointer!\n");

    Elf64_Phdr *ph = (Elf64_Phdr *) buffer;

    ph -> p_type    = PT_LOAD;
    ph -> p_offset  = offset;
    ph -> p_vaddr   = START_ADDRESS + offset;
    ph -> p_paddr   = ph -> p_vaddr;
    ph -> p_filesz  = file_size;
    ph -> p_memsz   = ph -> p_filesz;
    ph -> p_flags   = flags;
    ph -> p_align   = ALIGN;

    return sizeof(Elf64_Phdr);
}


size_t include_binary(const char *filename, uint8_t *buffer) {
    assert(filename && "Null pointer!\n");
    assert(buffer && "Null pointer!\n");

    int binary = open(filename, O_RDONLY);
    assert(binary != -1 && "Couldn't open binary file!\n");

    struct stat file_stat = {};
    assert(fstat(binary, &file_stat) >= 0 && "Can't get binary stat!\n");

    assert(read(binary, buffer, file_stat.st_size) != -1 && "Read failed!\n");

    return file_stat.st_size;
}


void clear_buffer(uint8_t *buffer, size_t buffer_size) {
    assert(memset(buffer, 0, buffer_size) && "Memset failed!\n");
}


size_t get_aligned_size(size_t size) {
    size_t mod = size % ALIGN;
    return (!mod) ? size : (size + ALIGN - mod);
}
