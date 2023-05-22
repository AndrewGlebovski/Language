#include <stdio.h>

#if defined(_WIN32) || defined(_WIN64)
    #include <io.h>
#elif __linux__
    #include <unistd.h>
#else
    #error "Your system case is not defined!\n"
#endif

#include <fcntl.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>
#include "libs/tree.hpp"
#include "libs/stack.hpp"
#include "libs/text.hpp"
#include "program.hpp"
#include "inter.hpp"
#include "elf_output.hpp"
#include "my_stdlib.hpp"


/// Main function of the program
#define FUNC_MAIN "VAR_22B14C_01B8923B"

/// Code offset in assembler output
const int TAB_SIZE = 4;

/// Intermediate representation initial capacity
const size_t IR_INIT_CAPACITY = 512;


typedef stack_data_t Variable;
typedef stack_data_t Function;
typedef unsigned long long hash_t;


struct VarList {
    Stack list = {};
    VarList *prev = nullptr;
};


#define ASSERT(condition, ...)          \
do                                      \
{                                       \
    if (!(condition)) {                 \
        printf("[%-p] ", node);         \
        printf(__VA_ARGS__);            \
        exit(1);                        \
    }                                   \
} while (0)

/// Prints command on new line without tab
#define PRINT(...)                      \
do {                                    \
    fprintf(file, __VA_ARGS__);         \
    fputc('\n', file);                  \
    line++;                             \
} while(0)

/// Prints command on new line with tab
#define PRINTL(...)                                 \
do {                                                \
    fprintf(file, "%*s", TAB_SIZE, "");             \
    fprintf(file, __VA_ARGS__);                     \
    fputc('\n', file);                              \
    line++;                                         \
} while(0)

/// Prints empty line to the file
#define SKIP_LINE(...) do {                         \
    fputc('\n', file);                              \
    line++;                                         \
} while(0)

/// Calls function for assembler source code output with only argument
#define CALL_FUNC(func_name, node_arg) func_name(node_arg, file, var_list, ir)

/// Current line index in file
int line = 1;

/// List of the functions
Stack func_list = {};

/// Reads definition sequence type node and prints result to file
void read_def_sequence(const Node *node, FILE *file, VarList *var_list, IR *ir);

/// Reads sequence type node and prints result to file
void read_sequence(const Node *node, FILE *file, VarList *var_list, IR *ir);

/// Add new local variable to variable list of the current scope
void add_local_variable(const Node *node, FILE *file, VarList *var_list, IR *ir);

/// Add new global variable to variable list of the current scope
void add_global_variable(const Node *node, FILE *file, VarList *var_list, IR *ir);

/// Add new function to function list of the current scope
void add_function(const Node *node, FILE *file, VarList *var_list, IR *ir);

/// Prints expression to file
void add_expression(const Node *node, FILE *file, VarList *var_list, IR *ir);

/// Prints variable assign operation to file
void add_assign(const Node *node, FILE *file, VarList *var_list, IR *ir);

/// Prints if operator to file
void add_if(const Node *node, FILE *file, VarList *var_list, IR *ir);

/// Prints while operator to file
void add_while(const Node *node, FILE *file, VarList *var_list, IR *ir);

/// Prints function argument in reverse order and counts their amount
int add_function_call_arg(const Node *node, FILE *file, VarList *var_list, IR *ir);

/// Prints function call to file
void add_function_call(const Node *node, FILE *file, VarList *var_list, IR *ir);

/// Prints return statement to file
void add_return(const Node *node, FILE *file, VarList *var_list, IR *ir);


/**
 * \brief Finds variable in list by its name
 * \param [in] var_name     Var name
 * \param [in] var_list     List of variables
 * \param [in] is_global    Returns non zero value if variable was declarated global
 * \param [in] max_depth    Max depth of recursive search in var list previous
 * \return Pointer to variable or null
*/
Variable *find_variable(const char *var_name, const VarList *var_list, int *is_global = nullptr, int max_depth = INT_MAX);


/**
 * \brief Finds function by its name
 * \param [in] func_name    Function name
 * \return Pointer to function
*/
Function *find_function(const char *func_name);


/**
 * \brief Includes file content in assembler code
 * \param [in]  filename    Path to input file
 * \param [out] file        Output file pointer
*/
void include_file(const char *filename, FILE *file);


/**
 * \brief Counts local variables and parameters in function
 * \param [in] varlist      To start count from
 * \return Variables count
*/
int get_frame_size(const VarList *varlist);


/**
 * \brief Prints condition expression to the file
 * \param [in]  cmd_str     Conditional jmp string
 * \param [in]  cmd_opcode  Conditional jmp opcode
 * \param [out] file        Output stream
 * \param [out] ir          Intermediate representation
*/
void print_cond(const char *cmd_str, uint8_t cmd_opcode, FILE *file, IR *ir);


/**
 * \brief Initializes varlist stack and set previous one
 * \param [in] prev Previous VarList for this one
 * \return New VarList
*/
VarList init_varlist(VarList *prev = nullptr);


/**
 * \brief Destructs varlist stack
 * \param [in] varlist To free
*/
void free_varlist(VarList *varlist);


/**
 * \brief GNU Hash function for string
 * \param [in] str Char string
 * \return Hash sum
*/
hash_t gnu_hash(const char *str);




int print_program(const Tree *tree, const char *filename) {
    if (!tree) return 1;
    if (!filename) return 2;

    FILE *file = fopen(filename, "w");

    line = 1;

    VarList global_list = init_varlist();

    stack_constructor(&func_list, 2);

    IR ir = {};
    IR_constructor(&ir, IR_INIT_CAPACITY);

    Function lib[] = {
        {FUNC_IN,   gnu_hash(FUNC_IN),      IN_ARGS_NUMBER,     0.0, IN_OFFSET},
        {FUNC_OUT,  gnu_hash(FUNC_OUT),     OUT_ARGS_NUMBER,    0.0, OUT_OFFSET},
        {FUNC_SQRT, gnu_hash(FUNC_SQRT),    SQRT_ARGS_NUMBER,   0.0, SQRT_OFFSET},
    };

    ir.ip = STDLIB_SIZE;

    for (int i = 0; i < (int)(sizeof(lib) / sizeof(Function)); i++) stack_push(&func_list, lib[i]);

    PRINT("global _start");
    SKIP_LINE();

    SKIP_LINE();
    include_file("stdlib.s", file);

    PRINT("section .text");
    SKIP_LINE();

    PRINT("_start:");

    PRINTL("call %s", "FUNC_22B14C_01B8923B");
    create_jmp(&ir, CALL);

    PRINTL("mov rdi, rax"); // COPY MAIN EXIT CODE TO SYSCALL
    IR_add(&ir, MOV, create_reg(64, DI), create_reg(64, A));

    PRINTL("mov rax, 60");
    IR_add(&ir, MOV, create_reg(64, A), create_const(64, 60));

    PRINTL("syscall");
    IR_add(&ir, SYSCALL);

    SKIP_LINE();

    read_def_sequence(tree -> root, file, &global_list, &ir);

    if (global_list.list.size) {
        PRINT("section .data");
        SKIP_LINE();

        for (int i = 0; i < global_list.list.size; i++)
            PRINT("%s dq %d", global_list.list.data[i].name, (int)(global_list.list.data[i].init_value * 1000));
    }

    Function *main_func = find_function(FUNC_MAIN);

    if (!main_func) {
        printf("Main function was not declarated in the current scope!\n");
        abort();
    }

    set_jmp_rel_address(ir.cmds, (int32_t) main_func -> func_offset);

    // IR_dump(&ir, stdout);

    create_elf("my_elf.exe", &ir);

    IR_destructor(&ir);

    free_varlist(&global_list);

    stack_destructor(&func_list);

    fclose(file);

    return 0;
}


void include_file(const char *filename, FILE *file) {
    int origin = open(filename, O_RDONLY);

    Text text = {};
    read_text(&text, origin);

    PRINT("; Included from %s", filename);
    
    for (String *i = text.lines; i -> str; i++) PRINT("%s", i -> str);

    close(origin);
}


void read_def_sequence(const Node *node, FILE *file, VarList *var_list, IR *ir) {
    for (const Node *iter = node; iter; iter = iter -> right){
        ASSERT(iter, "Definition sequence is null!\n");

        ASSERT(iter -> type == TYPE_DEF_SEQ, "Definition sequence expect type %i, but %i got!\n", TYPE_DEF_SEQ, iter -> type);

        ASSERT(iter -> left, "Definition sequence has no left child!\n");

        switch (iter -> left -> type) {
            case TYPE_NVAR:     CALL_FUNC(add_global_variable, iter -> left);   break;
            case TYPE_DEF:      CALL_FUNC(add_function, iter -> left);          break;
            default: ASSERT(0, "Definition sequence left child has type %i!\n", iter -> left -> type);
        }

        SKIP_LINE();
    }
}


void read_sequence(const Node *node, FILE *file, VarList *var_list, IR *ir) {
    for (const Node *iter = node; iter; iter = iter -> right){
        ASSERT(iter, "Sequence is null!\n");

        ASSERT(iter -> type == TYPE_SEQ, "Sequence expect type %i, but %i got!\n", TYPE_SEQ, iter -> type);

        ASSERT(iter -> left, "Sequence has no left child!\n");

        switch (iter -> left -> type) {
            case TYPE_NVAR:     CALL_FUNC(add_local_variable, iter -> left);                      break;
            case TYPE_OP:       CALL_FUNC(add_assign, iter -> left);                              break;
            case TYPE_IF:       CALL_FUNC(add_if, iter -> left);                                  break;
            case TYPE_WHILE:    CALL_FUNC(add_while, iter -> left);                               break;
            case TYPE_RET:      CALL_FUNC(add_return, iter -> left);                              break;
            case TYPE_CALL:
                CALL_FUNC(add_function_call, iter -> left);

                PRINTL("add rsp, 8");
                IR_add(ir, ADD, create_reg(64, SP), create_const(64, 8));
                
                break;
            default: ASSERT(0, "Sequence left child has type %i!\n", iter -> left -> type);
        }
    }
}


void add_local_variable(const Node *node, FILE *file, VarList *var_list, IR *ir) {
    ASSERT(node -> type == TYPE_NVAR, "Node is not new variable type!\n");
    ASSERT(node -> right, "New variable has no expression to assign!\n");

    ASSERT(!find_variable(node -> value.var, var_list, nullptr, 1), "Variable %s has already been declarated!\n", node -> value.var);

    Variable new_var = {node -> value.var, gnu_hash(node -> value.var), -1 - get_frame_size(var_list), 0, 0};

    stack_push(&var_list -> list, new_var);

    CALL_FUNC(add_expression, node -> right); // THERE IS NO NEED TO POP VALUE CAUSE IT'S ALREADY ON STACK
}


void add_global_variable(const Node *node, FILE *file, VarList *var_list, IR *ir) {
    ASSERT(node -> type == TYPE_NVAR, "Node is not new variable type!\n");
    ASSERT(node -> right, "New variable has no expression to assign!\n");
    ASSERT(node -> right -> type == TYPE_NUM, "Global variables can only have constant initial value!\n");

    ASSERT(!find_variable(node -> value.var, var_list, nullptr, 1), "Variable %s has already been declarated!\n", node -> value.var);

    Variable new_var = {node -> value.var, gnu_hash(node -> value.var), get_frame_size(var_list), node -> right -> value.dbl, 0};

    stack_push(&var_list -> list, new_var);
}


void add_function(const Node *node, FILE *file, VarList *var_list, IR *ir) {
    ASSERT(node -> type == TYPE_DEF, "Node is not function define type!\n");
    ASSERT(node -> right, "Function has no sequence!\n");

    ASSERT(!find_function(node -> value.var), "Function %s has already been declarated!\n", node -> value.var);

    Function new_func = {node -> value.var, gnu_hash(node -> value.var), 0, 0, ir -> ip};

    PRINT("FUNC_%s:", node -> value.var + 4);

    PRINTL("push rbp");
    IR_add(ir, PUSH, create_reg(64, BP));

    PRINTL("mov rbp, rsp");
    IR_add(ir, MOV, create_reg(64, BP), create_reg(64, SP));

    VarList args_varlist = init_varlist(var_list);
    VarList local_varlist = init_varlist(&args_varlist);

    for (const Node *par = node -> left; par; par = par -> right, new_func.index++) 
        stack_push(&args_varlist.list, {par -> value.var, gnu_hash(par -> value.var), 2 + new_func.index, 0, 0});

    stack_push(&func_list, new_func);

    read_sequence(node -> right, file, &local_varlist, ir);

    free_varlist(&local_varlist);
    free_varlist(&args_varlist);

    SKIP_LINE();
}


void add_expression(const Node *node, FILE *file, VarList *var_list, IR *ir) {
    switch(node -> type) {
        case TYPE_NUM: {
            PRINTL("push %d", (int)(node -> value.dbl * 1000));
            IR_add(ir, PUSH, create_const(64, (int)(node -> value.dbl * 1000)));

            break;
        }
        case TYPE_VAR: {
            int is_global = 0;
            Variable *var = find_variable(node -> value.var, var_list, &is_global);

            ASSERT(var, "Variable %s is not declarated in the current scope!\n", node -> value.var);

            if (is_global) {
                PRINTL("push QWORD [%s]", var -> name);
                IR_add(ir, PUSH, create_mem(64, var -> index * 8));
            }
            else {
                PRINTL("push QWORD [rbp + (%i)]", var -> index * 8);
                IR_add(ir, PUSH, create_mem(64, var -> index * 8, {64, BP}));
            }

            break;
        }
        case TYPE_CALL: {
            CALL_FUNC(add_function_call, node);

            break;
        }
        case TYPE_OP: {
            if (node -> left)   CALL_FUNC(add_expression, node -> left);
            if (node -> right)  CALL_FUNC(add_expression, node -> right);

            switch (node -> value.op) {
                case OP_ADD: {
                    PRINTL("pop rdi");
                    IR_add(ir, POP, create_reg(64, DI));

                    PRINTL("pop rsi");
                    IR_add(ir, POP, create_reg(64, SI));

                    PRINTL("add rdi, rsi");
                    IR_add(ir, ADD, create_reg(64, DI), create_reg(64, SI));

                    PRINTL("push rdi");
                    IR_add(ir, PUSH, create_reg(64, DI));

                    break;
                }
                case OP_SUB: {
                    PRINTL("pop rsi");
                    IR_add(ir, POP, create_reg(64, SI));

                    PRINTL("pop rdi");
                    IR_add(ir, POP, create_reg(64, DI));

                    PRINTL("sub rdi, rsi");
                    IR_add(ir, SUB, create_reg(64, DI), create_reg(64, SI));

                    PRINTL("push rdi");
                    IR_add(ir, PUSH, create_reg(64, DI));

                    break;
                }
                case OP_MUL: {
                    PRINTL("pop rdi");
                    IR_add(ir, POP, create_reg(64, DI));

                    PRINTL("pop rax");
                    IR_add(ir, POP, create_reg(64, A));

                    PRINTL("imul rdi");
                    IR_add(ir, IMUL, create_reg(64, DI));

                    PRINTL("mov rdi, 1000");
                    IR_add(ir, MOV, create_reg(64, DI), create_const(64, 1000));

                    PRINTL("idiv rdi");
                    IR_add(ir, IDIV, create_reg(64, DI));
                    
                    PRINTL("cdqe");
                    IR_add(ir, CDQE);

                    PRINTL("push rax");
                    IR_add(ir, PUSH, create_reg(64, A));

                    break;
                }
                case OP_DIV: {
                    PRINTL("pop rdi");
                    IR_add(ir, POP, create_reg(64, DI));

                    PRINTL("pop rax");
                    IR_add(ir, POP, create_reg(64, A));
                    
                    PRINTL("mov rsi, 1000");
                    IR_add(ir, MOV, create_reg(64, SI), create_const(64, 1000));

                    PRINTL("imul rsi");
                    IR_add(ir, IMUL, create_reg(64, SI));

                    PRINTL("idiv rdi");
                    IR_add(ir, IDIV, create_reg(64, DI));

                    PRINTL("cdqe");
                    IR_add(ir, CDQE);

                    PRINTL("push rax");
                    IR_add(ir, PUSH, create_reg(64, A));

                    break;
                }

                case OP_EQ:     print_cond("je", JE, file, ir);     break;
                case OP_NEQ:    print_cond("jne", JNE, file, ir);   break;
                case OP_GRE:    print_cond("jg", JG, file, ir);     break;
                case OP_LES:    print_cond("jl", JL, file, ir);     break;
                case OP_GEQ:    print_cond("jge", JGE, file, ir);   break;
                case OP_LEQ:    print_cond("jle", JLE, file, ir);   break;

                default: ASSERT(0, "Unexpected operator %i in expression!\n", node -> value.op);
            }

            break;
        }
        default: ASSERT(0, "Node has type %i and it's not expression type!\n", node -> type);
    }
}


void add_assign(const Node *node, FILE *file, VarList *var_list, IR *ir) {
    ASSERT(node -> type == TYPE_OP && node -> value.op == OP_ASS, "Assign expect op %i, but %i got!\n", OP_ASS, node -> value.op);
    ASSERT(node -> left, "No variable to assign!\n");
    ASSERT(node -> left -> type == TYPE_VAR, "Can't assign value to this node!\n");
    ASSERT(node -> right, "No expression to assign!\n");

    CALL_FUNC(add_expression, node -> right);
    
    int is_global = 0;
    Variable *var = find_variable(node -> left -> value.var, var_list, &is_global);

    ASSERT(var, "Variable %s is not declarated in the current scope!\n", node -> left -> value.var);

    if (is_global) {
        PRINTL("pop QWORD [%s]", var -> name);
        IR_add(ir, POP, create_mem(64, var -> index * 8));
    }
    else {
        PRINTL("pop QWORD [rbp + (%i)]", var -> index * 8);
        IR_add(ir, POP, create_mem(64, var -> index * 8, {64, BP}));
    }
}


void add_if(const Node *node, FILE *file, VarList *var_list, IR *ir) {
    ASSERT(node -> type == TYPE_IF, "If expect type %i, but %i got!\n", TYPE_IF, node -> type);
    ASSERT(node -> left, "If has no condition!\n");
    ASSERT(node -> right, "If has no branch!\n");
    ASSERT(node -> right -> left, "If has no sequence after it!\n");

    CALL_FUNC(add_expression, node -> left);

    PRINTL("pop rdi");
    IR_add(ir, POP, create_reg(64, DI));

    PRINTL("test rdi, rdi");
    IR_add(ir, TEST, create_reg(64, DI), create_reg(64, DI));

    int cur_line = line;

    size_t jmp_false_index = ir -> size;

    PRINTL("je IF_%i_FALSE", cur_line);
    create_jmp(ir, JE);

    VarList new_varlist = init_varlist(var_list);
    read_sequence(node -> right -> left, file, &new_varlist, ir);
    free_varlist(&new_varlist);

    if (node -> right -> right) {
        size_t jmp_end_index = ir -> size;

        PRINTL("jmp IF_%i_END", cur_line);
        create_jmp(ir, JMP);

        PRINT("IF_%i_FALSE:", cur_line);
        set_jmp_rel_address(ir -> cmds + jmp_false_index, (int32_t) ir -> ip);

        new_varlist = init_varlist(var_list);
        read_sequence(node -> right -> right, file, &new_varlist, ir);
        free_varlist(&new_varlist);

        PRINT("IF_%i_END:", cur_line);
        set_jmp_rel_address(ir -> cmds + jmp_end_index, (int32_t) ir -> ip);
    }
    else {
        PRINT("IF_%i_FALSE:", cur_line);
        set_jmp_rel_address(ir -> cmds + jmp_false_index, (int32_t) ir -> ip);
    }
}


void add_while(const Node *node, FILE *file, VarList *var_list, IR *ir) {
    ASSERT(node -> type == TYPE_WHILE, "While expect type %i, but %i got!\n", TYPE_WHILE, node -> type);
    ASSERT(node -> right, "While has no sequence after it!\n");
    ASSERT(node -> left, "While has no condition!\n");

    int cur_line = line;

    size_t jmp_index = ir -> size;

    PRINTL("jmp CYCLE_%i_CONDITION", cur_line);
    create_jmp(ir, JMP);

    PRINT("CYCLE_%i:", cur_line);
    size_t cycle_ip = ir -> ip;

    VarList new_varlist = init_varlist(var_list);
    read_sequence(node -> right, file, &new_varlist, ir);
    free_varlist(&new_varlist);

    PRINT("CYCLE_%i_CONDITION:", cur_line);
    set_jmp_rel_address(ir -> cmds + jmp_index, (int32_t) ir -> ip);

    CALL_FUNC(add_expression, node -> left);

    PRINTL("pop rdi");
    IR_add(ir, POP, create_reg(64, DI));

    PRINTL("test rdi, rdi");
    IR_add(ir, TEST, create_reg(64, DI), create_reg(64, DI));

    PRINTL("jne CYCLE_%i", cur_line);
    create_jmp(ir, JNE);
    set_jmp_rel_address(ir -> cmds + ir -> size - 1, (int32_t) cycle_ip);
}


int add_function_call_arg(const Node *node, FILE *file, VarList *var_list, IR *ir) {
    int arg_count = 0;

    if (node -> right) arg_count = CALL_FUNC(add_function_call_arg, node -> right);

    CALL_FUNC(add_expression, node -> left);

    return arg_count + 1;
}


void add_function_call(const Node *node, FILE *file, VarList *var_list, IR *ir) {
    ASSERT(node -> type == TYPE_CALL, "Function call expect type %i, but %i got!\n", TYPE_CALL, node -> type);

    Function *func = find_function(node -> value.var);

    ASSERT(func, "Function %s was not declarated in the current scope!\n", node -> value.var);

    int arg_count = 0;
    if (node -> left) arg_count = CALL_FUNC(add_function_call_arg, node -> left);

    ASSERT(arg_count == func -> index, "Function %s expects %i arguments, but got %i!\n", func -> name, func -> index, arg_count);

    PRINTL("call FUNC_%s", node -> value.var + 4);
    create_jmp(ir, CALL);
    set_jmp_rel_address(ir -> cmds + ir -> size - 1, (int32_t) func -> func_offset);

    if (func -> index) {
        PRINTL("add rsp, %d", func -> index * 8);
        IR_add(ir, ADD, create_reg(64, SP), create_const(64, func -> index * 8));
    }

    PRINTL("push rax");
    IR_add(ir, PUSH, create_reg(64, A));
}


void add_return(const Node *node, FILE *file, VarList *var_list, IR *ir) {
    ASSERT(node -> type == TYPE_RET, "Return expect type %i, but %i got!\n", TYPE_RET, node -> type);
    ASSERT(node -> left, "Return has no expression!\n");

    CALL_FUNC(add_expression, node -> left);

    PRINTL("pop rax");
    IR_add(ir, POP, create_reg(64, A));

    PRINTL("mov rsp, rbp");
    IR_add(ir, MOV, create_reg(64, SP), create_reg(64, BP));

    PRINTL("pop rbp");
    IR_add(ir, POP, create_reg(64, BP));

    PRINTL("ret");
    IR_add(ir, RET);
}


Variable *find_variable(const char *var_name, const VarList *var_list, int *is_global, int max_depth) {
    assert(var_name && "Null pointer!\n");

    hash_t hash = gnu_hash(var_name);

    for(int d = 1; d <= max_depth && var_list; d++, var_list = var_list -> prev) {
        for(int i = 0; i < var_list -> list.size; i++) {
            if ((var_list -> list.data)[i].hash == hash && !strcmp((var_list -> list.data)[i].name, var_name)) {
                if (is_global) *is_global = (var_list -> prev == nullptr);

                return var_list -> list.data + i;
            }
        }
    }

    return nullptr;
}


Function *find_function(const char *func_name) {
    assert(func_name && "Null pointer!\n");

    hash_t hash = gnu_hash(func_name);

    for (int i = 0; i < func_list.size; i++)
        if (func_list.data[i].hash == hash && !strcmp(func_list.data[i].name, func_name)) 
            return func_list.data + i;
    
    return nullptr;
}


hash_t gnu_hash(const char *str) {
    assert(str && "Null pointer!\n");
    
    hash_t hash = 5381;
    while (*str) hash = hash * 33 + (hash_t) *str++;
    
    return hash;
}


void print_cond(const char *cmd_str, uint8_t cmd_opcode, FILE *file, IR *ir) {
    PRINTL("pop rsi");
    IR_add(ir, POP, create_reg(64, SI));

    PRINTL("pop rdi");
    IR_add(ir, POP, create_reg(64, DI));

    PRINTL("cmp rdi, rsi");
    IR_add(ir, CMP, create_reg(64, DI), create_reg(64, SI));

    PRINTL("mov rax, 1000");
    IR_add(ir, MOV, create_reg(64, A), create_const(64, 1000));

    size_t jmp_index = ir -> size;

    PRINTL("%s COND_%i", cmd_str, line);
    create_jmp(ir, cmd_opcode);

    PRINTL("xor rax, rax");
    IR_add(ir, XOR, create_reg(64, A), create_reg(64, A));

    PRINT("COND_%i:", line - 2);
    set_jmp_rel_address(ir -> cmds + jmp_index, (int32_t) ir -> ip);

    PRINTL("push rax");
    IR_add(ir, PUSH, create_reg(64, A));
}


VarList init_varlist(VarList *prev) {
    VarList varlist = {{}, prev};
    
    stack_constructor(&varlist.list, 2);
    
    return varlist;
}


void free_varlist(VarList *varlist) {
    stack_destructor(&varlist -> list);
}


int get_frame_size(const VarList *varlist) {
    if (!varlist -> prev) return 0;
    if (!varlist -> prev -> prev) return 0;

    int count = 0;
    
    for(const VarList *iter = varlist; iter -> prev -> prev; iter = iter -> prev)
        count += iter -> list.size;

    return count;
}
