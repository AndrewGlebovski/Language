#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <stdint.h>
#include "inter.hpp"




#define ASSERT(condition, exitcode, ...)    \
do {                                        \
    if (!(condition)) {                     \
        printf(__VA_ARGS__);                \
        return exitcode;                    \
    }                                       \
} while (0)


#define VERIFY(ir_ptr)                      \
do {                                        \
    int exitcode = IR_verifier(ir_ptr);     \
    if (exitcode) {                         \
        printf("%s(%d): %s", __FILE__,      \
            __LINE__, __PRETTY_FUNCTION__); \
        return exitcode;                    \
    }                                       \
} while (0)




/**
 * \brief Checks intermediate representation for problems
 * \param [in] ir Intermediate representation
 * \return Non zero value means error
*/
int IR_verifier(const IR *ir);


/**
 * \brief Resizes intermediate representation buffer
 * \param [in] ir Intermediate representation
 * \return Non zero value means error
*/
int IR_resize(IR *ir);


/**
 * \brief Prints argument of register type to stream
 * \param [in]  reg_arg Register argument
 * \param [out] file    Output stream
 * \note Does not show R8-R15 registers properly
*/
void print_register_argument(const arg_reg_t *reg_arg, FILE *file);


/**
 * \brief Prints argument of memory reference type to stream
 * \param [in]  mem_arg Register argument
 * \param [out] file    Output stream
*/
void print_memory_argument(const arg_mem_t *mem_arg, FILE *file);




int IR_constructor(IR *ir, size_t capacity) {
    ASSERT(ir, 1, "Can't construct with nullptr!\n");
    ASSERT(capacity, 1, "Can't construct with zero capacity!\n");

    ir -> cmds = (AsmCmd *) calloc(capacity, sizeof(AsmCmd));
    ASSERT(ir -> cmds, 1, "Couldn't allocate buffer for commands!\n");

    ir -> capacity = capacity;
    ir -> size = 0;
    ir -> ip = 0;

    return 0;
}


int IR_add(IR *ir, uint8_t code, AsmArg arg1, AsmArg arg2) {
    VERIFY(ir);

    ir -> cmds[ir -> size].code = code;
    ir -> cmds[ir -> size].arg1 = arg1;
    ir -> cmds[ir -> size].arg2 = arg2;

    ir -> ip += get_command_size(ir -> cmds + ir -> size);
    ir -> size++;

    return IR_resize(ir);
}


int IR_destructor(IR *ir) {
    VERIFY(ir);

    free(ir -> cmds);
    ir -> cmds = nullptr;

    ir -> capacity = 0;
    ir -> size = 0;
    ir -> ip = 0;

    return 0;
}


int IR_verifier(const IR *ir) {
    ASSERT(ir,              1, "IR is nullptr!\n");
    ASSERT(ir -> capacity,  2, "IR has zero capacity!\n");
    ASSERT(ir -> cmds,      3, "IR with zero size!\n");

    return 0;
}


int IR_resize(IR *ir) {
    VERIFY(ir);

    if (ir -> size < ir -> capacity) return 0;

    size_t new_capacity = ir -> capacity * 2;

    ir -> cmds = (AsmCmd *) realloc(ir -> cmds, new_capacity * sizeof(AsmCmd));
    ASSERT(ir -> cmds, 1, "Couldn't reallocate buffer for commands!\n");

    for(size_t i = ir -> capacity; i < new_capacity; i++) ir -> cmds[i] = {};

    ir -> capacity = new_capacity;

    return 0;
}


int IR_dump(const IR *ir, FILE *stream) {
    VERIFY(ir);

    fprintf(stream, "IR[%p]:\n", ir);
    fprintf(stream, "  Capacity:       %lu\n", ir -> capacity);
    fprintf(stream, "  Commands count: %lu\n", ir -> size);
    fprintf(stream, "  Code size:      %lu\n", ir -> ip);
    fprintf(stream, "  Commands[%p]:\n", ir -> cmds);

    size_t ip = 0;

    for (size_t i = 0; i < ir -> size; i++) {
        fprintf(stream, "    [%06lu] ", ip);
        print_command(ir -> cmds + i, stream);
        ip += get_command_size(ir -> cmds + i);
    }

    putchar('\n');

    return 0;
}


size_t write_command(const AsmCmd *cmd, int file) {
    ASSERT(cmd, 0, "Command is nullptr!\n");
    ASSERT(file > -1, 0, "File descriptor is -1!\n");

    return 8;
}


void print_command(const AsmCmd *cmd, FILE *file) {
    ASSERT(cmd,, "Command is nullptr!\n");
    ASSERT(file,, "File descriptor is nullptr!\n");

    switch (cmd -> code) {
        case MOV:       fprintf(file, "mov "); break;
        case PUSH:      fprintf(file, "push "); break;
        case POP:       fprintf(file, "pop "); break;
        case ADD:       fprintf(file, "add "); break;
        case SUB:       fprintf(file, "sub "); break;
        case IMUL:      fprintf(file, "imul "); break;
        case IDIV:      fprintf(file, "idiv "); break;
        case AND:       fprintf(file, "and "); break;
        case OR:        fprintf(file, "or "); break;
        case XOR:       fprintf(file, "xor "); break;
        case SHR:       fprintf(file, "shr "); break;
        case SHL:       fprintf(file, "shl "); break;
        case INC:       fprintf(file, "inc "); break;
        case DEC:       fprintf(file, "dec "); break;
        case CALL:      fprintf(file, "call "); break;
        case RET:       fprintf(file, "ret "); break;
        case CDQE:      fprintf(file, "cdqe"); break;
        case LEA:       fprintf(file, "lea "); break;
        case TEST:      fprintf(file, "test "); break;
        case CMP:       fprintf(file, "cmp "); break;
        case JMP:       fprintf(file, "jmp "); break;
        case JE:        fprintf(file, "je "); break;
        case JNE:       fprintf(file, "jne "); break;
        case JG:        fprintf(file, "jg "); break;
        case JGE:       fprintf(file, "jge "); break;
        case JL:        fprintf(file, "jl "); break;
        case JLE:       fprintf(file, "jle "); break;
        case JA:        fprintf(file, "ja "); break;
        case JAE:       fprintf(file, "jae "); break;
        case JB:        fprintf(file, "jb "); break;
        case JBE:       fprintf(file, "jbe "); break;
        case SYSCALL:   fprintf(file, "syscall"); break;
        default:        fprintf(file, "[UKNOWN COMMAND]"); return;
    }

    if (cmd -> arg1.type != NONE) {
        print_argument(&cmd -> arg1, file);

        if (cmd -> arg2.type != NONE) {
            fprintf(file, ", ");
            print_argument(&cmd -> arg2, file);
        }
    }

    fputc('\n', file);
}


void print_argument(const AsmArg *arg, FILE *file) {
    switch (arg -> type) {
        case NONE:  fprintf(file, "[NONE]"); break;
        case REG:   print_register_argument(&arg -> value.reg_arg, file); break;
        case MEM:   print_memory_argument(&arg -> value.mem_arg, file); break;
        case CONST: fprintf(file, "%ld", arg -> value.const_arg.value); break;
        default:    fprintf(file, "[INVALID ARG TYPE]"); break;
    }
}


void print_register_argument(const arg_reg_t *reg_arg, FILE *file) {
    // NOTE: DOES NOT SHOW R8-R15 PROPERLY

    if (reg_arg -> size == 64 || reg_arg -> reg_id >= R8) fputc('r', file);
    else if (reg_arg -> size == 32) fputc('e', file);
    
    switch (reg_arg -> reg_id) {
        case A:     fprintf(file, "a"); break;
        case B:     fprintf(file, "b"); break;
        case C:     fprintf(file, "c"); break;
        case D:     fprintf(file, "d"); break;
        case SI:    fprintf(file, "si"); break;
        case DI:    fprintf(file, "di"); break;
        case BP:    fprintf(file, "bp"); break;
        case SP:    fprintf(file, "sp"); break;
        case R8:    fprintf(file, "8"); break;
        case R9:    fprintf(file, "9"); break;
        case R10:   fprintf(file, "10"); break;
        case R11:   fprintf(file, "11"); break;
        case R12:   fprintf(file, "12"); break;
        case R13:   fprintf(file, "13"); break;
        case R14:   fprintf(file, "14"); break;
        case R15:   fprintf(file, "15"); break;
        default: fprintf(file, "[UNKNOWN REGISTER]"); return;
    }

    if (reg_arg -> reg_id < R8) {
        if (reg_arg -> size >= 16 && reg_arg -> reg_id < SI) {
            fputc('x', file);
        }
        else if (reg_arg -> size == 8) {
            if (reg_arg -> high_bits) fputc('h', file);
            else fputc('l', file);
        }
    }
}


void print_memory_argument(const arg_mem_t *mem_arg, FILE *file) {
    switch (mem_arg -> size) {
        case 8: fprintf(file, "BYTE"); break;
        case 16: fprintf(file, "WORD"); break;
        case 32: fprintf(file, "DWORD"); break;
        case 64: fprintf(file, "QWORD"); break;
        default: fprintf(file, "[INVALID SIZE]"); return;
    }

    fprintf(file, "[%d", mem_arg -> displace);

    if (mem_arg -> base.size) {
        fprintf(file, "+");
        print_register_argument(&mem_arg -> base, file);
    }

    if (mem_arg -> index.size) {
        fprintf(file, "+");
        print_register_argument(&mem_arg -> index, file);

        if (mem_arg -> scale.size) {
            fprintf(file, "*");
            print_register_argument(&mem_arg -> scale, file);
        }
    }

    fprintf(file, "]");
}


size_t get_command_size(const AsmCmd *cmd) {
    ASSERT(cmd, 0, "Command is nullptr!\n");

    return 8;
}


AsmArg create_reg(uint8_t size, uint8_t reg_id, uint8_t high_bits) {
    AsmArg arg = {};

    arg.type = REG;
    arg.value.reg_arg = {size, reg_id, high_bits};

    return arg;
}

AsmArg create_const(uint8_t size, int64_t value) {
    AsmArg arg = {};

    arg.type = CONST;
    arg.value.const_arg = {size, value};

    return arg;
}

AsmArg create_mem(uint8_t size, int32_t displace, arg_reg_t base, arg_reg_t index, arg_reg_t scale) {
    AsmArg arg = {};

    arg.type = MEM;
    arg.value.mem_arg = {size, base, index, scale, displace};

    return arg;
}


void set_jmp_address(AsmCmd *cmd, int32_t address) {
    cmd -> arg1.value.const_arg.value = address;
}


void set_jmp_rel_address(AsmCmd *cmd, int32_t address) {
    cmd -> arg1.value.const_arg.value = address - cmd -> arg1.value.const_arg.value;
}


void create_jmp(IR *ir, uint8_t cmd) {
    IR_add(ir, cmd, create_const(32, 0));
    set_jmp_address(ir -> cmds + ir -> size - 1, (int32_t) ir -> ip);
}
