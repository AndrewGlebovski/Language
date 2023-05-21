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
 * \brief Writes command byte code in two 64-bit register-type operands pattern
 * \param [in]  opcode  Assembler command opcode
 * \param [in]  reg1    First register argument
 * \param [in]  reg2    Second register argument
 * \param [out] buffer  Buffer to write byte code in
 * \return Command size in bytes
*/
size_t write_2regs64_pattern(uint8_t opcode, arg_reg_t *reg1, arg_reg_t *reg2, uint8_t *buffer);


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
    uint8_t buffer[16] = "";

    for (size_t i = 0; i < ir -> size; i++) {
        
        fprintf(stream, "    [%06lu]", ip);

        size_t size = write_command(ir -> cmds + i, buffer);

        for(size_t j = 0; j < size; j ++) fprintf(stream, " %02hhx", buffer[j]);

        fprintf(stream, "\n             ");

        print_command(ir -> cmds + i, stream);

        fputc('\n', stream);

        ip += size;
    }

    putchar('\n');

    return 0;
}


int IR_write(const IR *ir, uint8_t *buffer) {
    VERIFY(ir);

    size_t ip = 0;
    for (size_t i = 0; i < ir -> size; i++)
        ip += write_command(ir -> cmds + i, buffer + ip);

    return 0;
}




#define PUSH_BYTE(byte) buffer[size++] = byte

#define PUSH_INT(integer) *(int32_t *)(buffer + size) = (int32_t) integer; size += 4

#define COMBINE_CMD(cmd_code, args_size, arg1_type, arg2_type) \
    (((uint32_t) cmd_code << 24) | ((uint32_t) args_size << 16) | ((uint32_t) arg1_type << 8) | (uint32_t) arg2_type)

#define COMBINE_BYTES(upper_byte, lower_byte) (((upper_byte) << 4U) | (lower_byte))

#define COMBINE_REG11(reg1, reg2) (0b11000000 | ((reg1) << 3U) | (reg2))

#define COMBINE_REG10(reg1, reg2) (0b10000000 | ((reg1) << 3U) | (reg2))

#define REG_ID(asm_arg) asm_arg.value.reg_arg.reg_id

#define R8_CHECK(reg_id, prefix)    \
if (reg_id >= R8) {                 \
    reg_id -= 0x8;                  \
    PUSH_BYTE(prefix);              \
}


size_t write_2regs64_pattern(uint8_t opcode, arg_reg_t *reg1, arg_reg_t *reg2, uint8_t *buffer) {
    size_t size = 0;

    if (reg1 -> reg_id <= DI && reg2 -> reg_id <= DI) {
        PUSH_BYTE(0x48);
    }
    else if (reg1 -> reg_id <= DI && reg2 -> reg_id > DI) {
        reg2 -> reg_id -= 0x8;
        PUSH_BYTE(0x4c);
    }
    else if (reg1 -> reg_id > DI && reg2 -> reg_id <= DI) {
        reg1 -> reg_id -= 0x8;
        PUSH_BYTE(0x49);
    }
    else {
        reg1 -> reg_id -= 0x8;
        reg2 -> reg_id -= 0x8;
        PUSH_BYTE(0x4d);
    }

    PUSH_BYTE(opcode);
    PUSH_BYTE(COMBINE_REG11(reg2 -> reg_id, reg1 -> reg_id));

    return size;
}


size_t write_command(const AsmCmd *cmd, uint8_t *buffer) {
    ASSERT(cmd, 0, "Command is nullptr!\n");
    ASSERT(buffer, 0, "Buffer is nullptr!\n");

    size_t size = 0, args_size = cmd -> arg1.value.const_arg.size;

    AsmArg arg1 = cmd -> arg1, arg2 = cmd -> arg2;

    switch (COMBINE_CMD(cmd -> code, args_size, arg1.type, arg2.type)) {
        case COMBINE_CMD(MOV, 64, REG, REG):
            size += write_2regs64_pattern(0x89, &arg1.value.reg_arg, &arg2.value.reg_arg, buffer);
            break;
        
        case COMBINE_CMD(MOV, 64, REG, CONST):
            R8_CHECK(REG_ID(arg1), 0x49)
            else PUSH_BYTE(0x48);

            PUSH_BYTE(0xc7);
            PUSH_BYTE(COMBINE_REG11(0, REG_ID(arg1)));
            PUSH_INT(arg2.value.const_arg.value);
            break;

        case COMBINE_CMD(PUSH, 64, REG, NONE):
            R8_CHECK(REG_ID(arg1), 0x41);

            PUSH_BYTE(COMBINE_BYTES(0x5, REG_ID(arg1)));
            break;
        
        case COMBINE_CMD(PUSH, 64, CONST, NONE):
            PUSH_BYTE(0x68);
            PUSH_INT(arg1.value.const_arg.value);
            break;

        case COMBINE_CMD(PUSH, 64, MEM, NONE):
            if (arg1.value.mem_arg.base.size) {
                R8_CHECK(arg1.value.mem_arg.base.reg_id, 0x41);

                PUSH_BYTE(0xff);
                PUSH_BYTE(COMBINE_REG10(0b110, arg1.value.mem_arg.base.reg_id));

                // SCALE AND INDEX REGISTERS ARE NOT SUPPORTED
            }
            else {
                PUSH_BYTE(0xff);
                PUSH_BYTE(0x04);
                PUSH_BYTE(0x25);
            }

            PUSH_INT(arg1.value.mem_arg.displace);
            break;

        case COMBINE_CMD(POP, 64, REG, NONE):
            R8_CHECK(REG_ID(arg1), 0x41);

            PUSH_BYTE(COMBINE_BYTES(0x5, REG_ID(arg1) + 0x8));
            break;

        case COMBINE_CMD(POP, 64, MEM, NONE):
            if (arg1.value.mem_arg.base.size) {
                R8_CHECK(arg1.value.mem_arg.base.reg_id, 0x41);

                PUSH_BYTE(0x8f);
                PUSH_BYTE(COMBINE_REG10(0b000, arg1.value.mem_arg.base.reg_id));

                // SCALE AND INDEX REGISTERS ARE NOT SUPPORTED
            }
            else {
                PUSH_BYTE(0x8f);
                PUSH_BYTE(0x04);
                PUSH_BYTE(0x25);
            }

            PUSH_INT(arg1.value.mem_arg.displace);
            break;

        case COMBINE_CMD(ADD, 64, REG, REG):
            size += write_2regs64_pattern(0x01, &arg1.value.reg_arg, &arg2.value.reg_arg, buffer);
            break;
        
        case COMBINE_CMD(ADD, 64, REG, CONST):
            R8_CHECK(REG_ID(arg1), 0x49)
            else PUSH_BYTE(0x48);

            PUSH_BYTE(0x81);
            PUSH_BYTE(COMBINE_REG11(0, REG_ID(arg1)));
            PUSH_INT(arg2.value.const_arg.value);
            break;
        
        case COMBINE_CMD(SUB, 64, REG, REG):
            size += write_2regs64_pattern(0x29, &arg1.value.reg_arg, &arg2.value.reg_arg, buffer);
            break;

        case COMBINE_CMD(IMUL, 64, REG, NONE):
            R8_CHECK(REG_ID(arg1), 0x49)
            else PUSH_BYTE(0x48);

            PUSH_BYTE(0xf7);
            PUSH_BYTE(COMBINE_REG11(0b101, REG_ID(arg1)));
            break;
        
        case COMBINE_CMD(IDIV, 64, REG, NONE):
            R8_CHECK(REG_ID(arg1), 0x49)
            else PUSH_BYTE(0x48);

            PUSH_BYTE(0xf7);
            PUSH_BYTE(COMBINE_REG11(0b101, REG_ID(arg1)));
            break;

        case COMBINE_CMD(AND, 64, REG, REG):
            size += write_2regs64_pattern(0x21, &arg1.value.reg_arg, &arg2.value.reg_arg, buffer);
            break;
        
        case COMBINE_CMD(OR, 64, REG, REG):
            size += write_2regs64_pattern(0x09, &arg1.value.reg_arg, &arg2.value.reg_arg, buffer);
            break;
        
        case COMBINE_CMD(XOR, 64, REG, REG):
            size += write_2regs64_pattern(0x31, &arg1.value.reg_arg, &arg2.value.reg_arg, buffer);
            break;

        case COMBINE_CMD(RET, 0, NONE, NONE):
            PUSH_BYTE(0xc3);
            break;

        case COMBINE_CMD(CDQE, 0, NONE, NONE):
            PUSH_BYTE(0x48);
            PUSH_BYTE(0x98);
            break;

        case COMBINE_CMD(TEST, 64, REG, REG):
            size += write_2regs64_pattern(0x85, &arg1.value.reg_arg, &arg2.value.reg_arg, buffer);
            break;
        
        case COMBINE_CMD(CMP, 64, REG, REG):
            size += write_2regs64_pattern(0x39, &arg1.value.reg_arg, &arg2.value.reg_arg, buffer);
            break;

        case COMBINE_CMD(JMP, 32, CONST, NONE):
            PUSH_BYTE(0xe9);
            PUSH_INT(arg1.value.const_arg.value);
            break;
        
        case COMBINE_CMD(JE, 32, CONST, NONE):
            PUSH_BYTE(0x0f);
            PUSH_BYTE(0x84);
            PUSH_INT(arg1.value.const_arg.value);
            break;
        
        case COMBINE_CMD(JNE, 32, CONST, NONE):
            PUSH_BYTE(0x0f);
            PUSH_BYTE(0x85);
            PUSH_INT(arg1.value.const_arg.value);
            break;
        
        case COMBINE_CMD(JG, 32, CONST, NONE):
            PUSH_BYTE(0x0f);
            PUSH_BYTE(0x8f);
            PUSH_INT(arg1.value.const_arg.value);
            break;
        
        case COMBINE_CMD(JGE, 32, CONST, NONE):
            PUSH_BYTE(0x0f);
            PUSH_BYTE(0x8d);
            PUSH_INT(arg1.value.const_arg.value);
            break;

        case COMBINE_CMD(JL, 32, CONST, NONE):
            PUSH_BYTE(0x0f);
            PUSH_BYTE(0x8c);
            PUSH_INT(arg1.value.const_arg.value);
            break;
        
        case COMBINE_CMD(JLE, 32, CONST, NONE):
            PUSH_BYTE(0x0f);
            PUSH_BYTE(0x8e);
            PUSH_INT(arg1.value.const_arg.value);
            break;
        
        case COMBINE_CMD(JA, 32, CONST, NONE):
            PUSH_BYTE(0x0f);
            PUSH_BYTE(0x87);
            PUSH_INT(arg1.value.const_arg.value);
            break;

        case COMBINE_CMD(JAE, 32, CONST, NONE):
            PUSH_BYTE(0x0f);
            PUSH_BYTE(0x83);
            PUSH_INT(arg1.value.const_arg.value);
            break;
        
        case COMBINE_CMD(JB, 32, CONST, NONE):
            PUSH_BYTE(0x0f);
            PUSH_BYTE(0x82);
            PUSH_INT(arg1.value.const_arg.value);
            break;

        case COMBINE_CMD(JBE, 32, CONST, NONE):
            PUSH_BYTE(0x0f);
            PUSH_BYTE(0x86);
            PUSH_INT(arg1.value.const_arg.value);
            break;

        case COMBINE_CMD(SYSCALL, 0, NONE, NONE):
            PUSH_BYTE(0x0f);
            PUSH_BYTE(0x05);
            break;

        case COMBINE_CMD(CALL, 32, CONST, NONE):
            PUSH_BYTE(0xe8);
            PUSH_INT(arg1.value.const_arg.value);
            break;

        default: 
            printf("Uknown command or operands combination: ");
            print_command(cmd, stdout);
            putc('\n', stdout);
            break;
    }

    return size;
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
        case CALL:      fprintf(file, "call "); break;
        case RET:       fprintf(file, "ret "); break;
        case CDQE:      fprintf(file, "cdqe"); break;
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
        if (reg_arg -> size >= 16 && reg_arg -> reg_id <= B) {
            fputc('x', file);
        }
        else if (reg_arg -> size == 8) {
            if (reg_arg -> high_bits) fputc('h', file);
            else fputc('l', file);
        }
    }
}


void print_memory_argument(const arg_mem_t *mem_arg, FILE *file) {
    ASSERT(mem_arg,, "Argument is null pointer!\n");
    ASSERT(file,, "File is null pointer!\n");

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
    uint8_t buffer[16] = "";
    return write_command(cmd, buffer);
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
    ASSERT(cmd,, "Null pointer!\n");
    cmd -> arg1.value.const_arg.value = address;
}


void set_jmp_rel_address(AsmCmd *cmd, int32_t address) {
    ASSERT(cmd,, "Null pointer!\n");
    cmd -> arg1.value.const_arg.value = address - cmd -> arg1.value.const_arg.value;
}


void create_jmp(IR *ir, uint8_t cmd) {
    IR_add(ir, cmd, create_const(32, 0));
    set_jmp_address(ir -> cmds + ir -> size - 1, (int32_t) ir -> ip);
}
