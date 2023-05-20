/**
 * \file
 * \brief Intermediate representation header file
*/


/// Types for AsmArg
typedef enum {
    NONE    = 0,
    REG     = 1,
    MEM     = 2,
    CONST   = 3,
} ARG_TYPES;


/// Registers IDs for arg_reg_t
typedef enum {
    A,
    C,
    D,
    B,
    SP,
    BP,
    SI,
    DI,
    R8,
    R9,
    R10,
    R11,
    R12,
    R13,
    R14,
    R15,
} REGISTER_ID;


typedef struct {
    uint8_t size        = 0;
    int64_t value       = 0;
} arg_const_t;


typedef struct {
    uint8_t size        = 0;
    uint8_t reg_id      = 0;
    uint8_t high_bits   = 0;
} arg_reg_t;


typedef struct {
    uint8_t size        = 0;
    arg_reg_t base      = {};
    arg_reg_t index     = {};
    arg_reg_t scale     = {};
    int32_t displace    = 0;
} arg_mem_t;


typedef union {
    arg_const_t const_arg = {};
    arg_reg_t   reg_arg;
    arg_mem_t   mem_arg;
} arg_t;


/// Holds argument for assembler command
typedef struct {
    uint8_t type = 0;
    arg_t value = {};
} AsmArg;


/// Assembler commands id
typedef enum {
    // Memory operations
    MOV     = 0x00,
    PUSH    = 0x01,
    POP     = 0x02,

    // Arithmetic operations
    ADD     = 0x03,
    SUB     = 0x04,
    IMUL    = 0x05,
    IDIV    = 0x06,

    // Bit operations
    AND     = 0x07,
    OR      = 0x08,
    XOR     = 0x09,
    // SHR     = 0x0A,
    // SHL     = 0x0B,
    
    // Increment
    // INC     = 0x0C,
    // DEC     = 0x0D,

    // Calls and rets
    CALL    = 0x0E,
    RET     = 0x0F,

    // Individual
    CDQE    = 0x10,
    // LEA     = 0x11,

    // Comparsion
    TEST    = 0x12,
    CMP     = 0x13,

    // Jumps (unconditional and conditional)
    JMP     = 0x14,
    JE      = 0x15,
    JNE     = 0x16,
    JG      = 0x17,
    JGE     = 0x18,
    JL      = 0x19,
    JLE     = 0x1A,
    JA      = 0x1B,
    JAE     = 0x1C,
    JB      = 0x1D,
    JBE     = 0x1E,

    // Syscall
    SYSCALL = 0x1F,
} COMMAND_ID;


/// Holds assembler command and its arguments
typedef struct {
    uint8_t code = 0;
    AsmArg arg1 = {};
    AsmArg arg2 = {};
} AsmCmd;


/// Program intermediate representation
typedef struct {
    AsmCmd *cmds = nullptr;
    size_t capacity = 0;
    size_t size = 0;
    size_t ip = 0;
} IR;


/**
 * \brief Initializes intermediate representation buffer
 * \param [out] ir          Intermediate representation
 * \param [in]  capacity    New buffer capacity
 * \return Non zero value means error
*/
int IR_constructor(IR *ir, size_t capacity);


/**
 * \brief Add assembler command to intermediate representation
 * \param [out] ir      Intermediate representation
 * \param [in]  code    Command code
 * \param [in]  arg1    First argument
 * \param [in]  arg1    Second argument
 * \return Non zero value means error
*/
int IR_add(IR *ir, uint8_t code, AsmArg arg1 = {}, AsmArg arg2 = {});


/**
 * \brief Destroys intermediate representation buffer
 * \param [out] ir      Intermediate representation
 * \return Non zero value means error
*/
int IR_destructor(IR *ir);


/**
 * \brief Prints intermediate representation buffer
 * \param [in]  ir      Intermediate representation
 * \param [out] stream  Output stream
 * \return Non zero value means error
*/
int IR_dump(const IR *ir, FILE *stream);


/**
 * \brief Writes command byte code to file
 * \param [in]  cmd     Assembler command
 * \param [out] buffer  Output buffer for opcode
 * \return Size of the command in bytes
*/
size_t write_command(const AsmCmd *cmd, uint8_t *buffer);


/**
 * \brief Prints command to stream
 * \param [in]  cmd     Assembler command
 * \param [out] file    Output stream
*/
void print_command(const AsmCmd *cmd, FILE *file);


/**
 * \brief Prints argument to stream
 * \param [in]  arg     Assembler argument
 * \param [out] file    Output stream
*/
void print_argument(const AsmArg *arg, FILE *file);


/**
 * \brief Returns size of the commands byte code
 * \param [in] cmd      Assembler command
 * \return Size of the command in bytes
*/
size_t get_command_size(const AsmCmd *cmd);


// TOOLS //


/// Creates assembler argument of register type
AsmArg create_reg(uint8_t size, uint8_t reg_id, uint8_t high_bits = 0);

/// Creates assembler argument of const type
AsmArg create_const(uint8_t size, int64_t value);

/// Creates assembler argument of memory reference type
AsmArg create_mem(uint8_t size, int32_t displace, arg_reg_t base = {}, arg_reg_t index = {}, arg_reg_t scale = {});

/// Sets first argument of jmp or call to the given address
void set_jmp_address(AsmCmd *cmd, int32_t address);

/// Sets first argument of jmp or call to relative address
/// Assumes that current jmp value is address of the next command after jmp
void set_jmp_rel_address(AsmCmd *cmd, int32_t address);

/// Creates jmp or call with address of the next command after jmp
void create_jmp(IR *ir, uint8_t cmd);
