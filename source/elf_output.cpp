#include <assert.h>
#include <elf.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include "inter.hpp"
#include "elf_output.hpp"



/**
 * \brief Fill elf header structure fields
 * \param [out] elf64   Elf64 header
*/
void create_elf64_header(Elf64_Ehdr *elf64);


/**
 * \brief Fill elf program header structure fields with IR information
 * \param [out] ph  Elf64 program header
 * \param [in]  ir  Intermediate representation
*/
void create_elf64_ph(Elf64_Phdr *ph, const IR *ir);


/**
 * \brief Writes elf information and bytecode to file
 * \param [in]  elf64   Elf64 header
 * \param [in]  ph      Elf64 program header
 * \param [in]  ir      Intermediate representation
 * \param [out] file    File desciptor
*/
void write_elf64(const Elf64_Ehdr *elf64, Elf64_Phdr *ph, const IR *ir, int file);




int create_elf(const char *elf_filename, const IR *ir) {
    assert(elf_filename && "Null pointer!\n");
    assert(ir && "Null pointer!\n");

    Elf64_Ehdr elf64 = {};
    create_elf64_header(&elf64);

    Elf64_Phdr ph = {};
    create_elf64_ph(&ph, ir);

    int file = open(elf_filename, O_WRONLY | O_CREAT, 00770);
    assert(file != -1 && "Couldn't open or creat such file!\n");

    write_elf64(&elf64, &ph, ir, file);

    close(file);

    return 0;
}


void create_elf64_header(Elf64_Ehdr *elf64) {
    assert(elf64 && "Null pointer!\n");

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
    elf64 -> e_entry                = 0x400000 + sizeof(Elf64_Ehdr) + sizeof(Elf64_Phdr);
    elf64 -> e_phoff                = sizeof(Elf64_Ehdr);
    elf64 -> e_shoff                = 0;
    elf64 -> e_version              = EV_CURRENT;
    elf64 -> e_flags                = 0;
    elf64 -> e_ehsize               = sizeof(Elf64_Ehdr);
    elf64 -> e_phentsize            = sizeof(Elf64_Phdr);
    elf64 -> e_phnum                = 1;
    elf64 -> e_shentsize            = 0;
    elf64 -> e_shnum                = 0;
    elf64 -> e_shstrndx             = SHN_UNDEF;
}


void create_elf64_ph(Elf64_Phdr *ph, const IR *ir) {
    assert(ph && "Null pointer!\n");
    assert(ir && "Null pointer!\n");

    ph -> p_type    = PT_LOAD;
    ph -> p_offset  = 0;
    ph -> p_vaddr   = 0x400000;
    ph -> p_paddr   = 0x400000;
    ph -> p_filesz  = ir -> ip + sizeof(Elf64_Ehdr) + sizeof(Elf64_Phdr);
    ph -> p_memsz   = ir -> ip + sizeof(Elf64_Ehdr) + sizeof(Elf64_Phdr);
    ph -> p_flags   = PF_X | PF_R | PF_W;
    ph -> p_align   = 0x1000;
}


void write_elf64(const Elf64_Ehdr *elf64, Elf64_Phdr *ph, const IR *ir, int file) {
    assert(elf64 && "Null pointer!\n");
    assert(ph && "Null pointer!\n");
    assert(ir && "Null pointer!\n");
    assert(file != -1 && "File descriptor is -1!\n");

    assert(write(file, elf64, sizeof(Elf64_Ehdr)) != -1 && "Write failed!\n");
    assert(write(file, ph, sizeof(Elf64_Phdr)) != -1 && "Write failed!\n");

    uint8_t *bytecode = (uint8_t *) calloc(ir -> ip, 1);
    assert(bytecode && "Couldn't allocate buffer for bytecode!\n");

    assert(!IR_write(ir, bytecode) && "IR write failed!\n");

    assert(write(file, bytecode, ir -> ip) != -1 && "Write failed!\n");

    free(bytecode);
}
