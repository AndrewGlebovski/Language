// STDLIB FILE //

#define         STDLIB_FILE "stdlib.bin"            ///< Path to standart library binary code
const size_t    STDLIB_SIZE = 0x14E;                ///< Size of the standart library binary file

// STDLIB FUNCTIONS //

#define         FUNC_IN "VAR_22B14C_00076DC0"       ///< "In" function real name
const int       IN_ARGS_NUMBER = 0;                 ///< Amount of arguments that "In" takes
const size_t    IN_OFFSET = 0;                      ///< "In" byte offset from the begin of the code section

#define         FUNC_OUT "VAR_22B14C_01435CD4"      ///< "Out" function real name
const int       OUT_ARGS_NUMBER = 1;                ///< Amount of arguments that "Out" takes
const size_t    OUT_OFFSET = 0x68;                  ///< "Out" byte offset from the begin of the code section

#define         FUNC_SQRT "VAR_22B14C_0062909C"     ///< "Sqrt" function real name
const int       SQRT_ARGS_NUMBER = 1;               ///< Amount of arguments that "Sqrt" takes
const size_t    SQRT_OFFSET = 0xee;                 ///< "Sqrt" byte offset from the begin of the code section
