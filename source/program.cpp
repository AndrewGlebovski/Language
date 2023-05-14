#include <stdio.h>

#if defined(_WIN32) || defined(_WIN64)
    #include <io.h>
#elif __linux__
    #include <unistd.h>
#else
    #error "Your system case is not defined!\n"
#endif

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "libs/tree.hpp"
#include "libs/stack.hpp"
#include "libs/text.hpp"
#include "program.hpp"


/// Code offset in assembler output
const int TAB_SIZE = 4;


typedef stack_data_t Variable;
typedef stack_data_t Function;


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
#define CALL_FUNC(func_name, node_arg) func_name(node_arg, file, var_list)




/// Current line index in file
int line = 1;

/// List of the functions
Stack func_list = {};


/// Reads definition sequence type node and prints result to file
void read_def_sequence(const Node *node, FILE *file, VarList *var_list);

/// Reads sequence type node and prints result to file
void read_sequence(const Node *node, FILE *file, VarList *var_list);

/// Add new local variable to variable list of the current scope
void add_local_variable(const Node *node, FILE *file, VarList *var_list);

/// Add new global variable to variable list of the current scope
void add_global_variable(const Node *node, FILE *file, VarList *var_list);

/// Add new function to function list of the current scope
void add_function(const Node *node, FILE *file, VarList *var_list);

/// Prints expression to file
void add_expression(const Node *node, FILE *file, VarList *var_list);

/// Prints variable assign operation to file
void add_assign(const Node *node, FILE *file, VarList *var_list);

/// Prints if operator to file
void add_if(const Node *node, FILE *file, VarList *var_list);

/// Prints while operator to file
void add_while(const Node *node, FILE *file, VarList *var_list);

/// Prints function call to file
void add_function_call(const Node *node, FILE *file, VarList *var_list);

/// Prints return statement to file
void add_return(const Node *node, FILE *file, VarList *var_list);


/**
 * \brief Finds variable in list by its name hash
 * \param [in] hash Var name hash
 * \param [in] var_list List of variables
 * \param [in] is_global Returns non zero value if variable was declarated global
 * \param [in] max_depth Max depth of recursive search in var list previous
 * \return Pointer to variable or null
*/
Variable *find_variable(size_t hash, const VarList *var_list, int *is_global = nullptr, int max_depth = INT_MAX);


/**
 * \brief Finds function by its name hash
 * \param [in] hash Function name hash
 * \return Pointer to function
*/
Function *find_function(size_t hash);


/**
 * \brief Includes file content in assembler code
 * \param [in]  filename    Path to input file
 * \param [out] file        Output file pointer
*/
void include_file(const char *filename, FILE *file);


/**
 * \brief Counts local variables and parameters in function
 * \param [in] varlist To start count from
 * \return Variables count
*/
int get_frame_size(const VarList *varlist);


/// Calculates hash sum of the string
size_t string_hash(const char *str);


/// Prints condition result to file
void print_cond(const char *cond_op, FILE *file);


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




int print_program(const Tree *tree, const char *filename) {
    if (!tree) return 1;
    if (!filename) return 2;

    FILE *file = fopen(filename, "w");

    line = 1;

    VarList global_list = init_varlist();

    stack_constructor(&func_list, 2);

    const char *MAIN_FUNC = "VAR_22B14C_01B8923B";

    Function lib[] = {
        {"VAR_22B14C_00076DC0", string_hash("VAR_22B14C_00076DC0"), 0},
        {"VAR_22B14C_01435CD4", string_hash("VAR_22B14C_01435CD4"), 1},
        {"VAR_22B14C_0062909C", string_hash("VAR_22B14C_0062909C"), 1},
    };

    for (int i = 0; i < (int)(sizeof(lib) / sizeof(Function)); i++) stack_push(&func_list, lib[i]);

    PRINT("global _start");
    SKIP_LINE();

    SKIP_LINE();
    include_file("stdlib.s", file);

    PRINT("section .text");
    SKIP_LINE();

    PRINT("_start:");
    PRINTL("call FUNC_22B14C_01B8923B");
    PRINTL("mov rdi, rax");                 /// Set exit code
    PRINTL("mov eax, 60");
    PRINTL("syscall");
    SKIP_LINE();

    read_def_sequence(tree -> root, file, &global_list);

    if (global_list.list.size) {
        PRINT("section .data");
        SKIP_LINE();

        for (int i = 0; i < global_list.list.size; i++)
            PRINT("%s dq 0", global_list.list.data[i].name);
    }

    if (!find_function(string_hash(MAIN_FUNC))) {
        printf("Main function was not declarated in the current scope!\n");
        abort();
    }

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


void read_def_sequence(const Node *node, FILE *file, VarList *var_list) {
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


void read_sequence(const Node *node, FILE *file, VarList *var_list) {
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
            case TYPE_CALL:     CALL_FUNC(add_function_call, iter -> left); PRINTL("add rsp, 8"); break;
            default: ASSERT(0, "Sequence left child has type %i!\n", iter -> left -> type);
        }
    }
}


void add_local_variable(const Node *node, FILE *file, VarList *var_list) {
    ASSERT(node -> type == TYPE_NVAR, "Node is not new variable type!\n");

    size_t hash = string_hash(node -> value.var);

    int is_global = 0;
    ASSERT(!find_variable(hash, var_list, &is_global, 1), "Variable %s has already been declarated!\n", node -> value.var);

    Variable new_var = {node -> value.var, hash, -1-get_frame_size(var_list)};
    stack_push(&var_list -> list, new_var);

    ASSERT(node -> right, "New variable has no expression to assign!\n");
    CALL_FUNC(add_expression, node -> right);
}


void add_global_variable(const Node *node, FILE *file, VarList *var_list) {
    ASSERT(node -> type == TYPE_NVAR, "Node is not new variable type!\n");

    size_t hash = string_hash(node -> value.var);

    int is_global = 0;
    ASSERT(!find_variable(hash, var_list, &is_global, 1), "Variable %s has already been declarated!\n", node -> value.var);

    Variable new_var = {node -> value.var, hash, 0};
    stack_push(&var_list -> list, new_var);

    if (node -> right) printf("Global variables always init with zero, expression discarded!\n");
}


void add_function(const Node *node, FILE *file, VarList *var_list) {
    ASSERT(node -> type == TYPE_DEF, "Node is not function define type!\n");
    
    ASSERT(!find_function(string_hash(node -> value.var)), "Function %s has already been declarated!\n", node -> value.var);

    Function new_func = {node -> value.var, string_hash(node -> value.var), 0};

    VarList args_varlist = init_varlist(var_list);
    VarList local_varlist = init_varlist(&args_varlist);

    PRINT("FUNC_%s:", node -> value.var + 4);
    PRINTL("push rbp");
    PRINTL("mov rbp, rsp");

    for (const Node *par = node -> left; par; par = par -> right, new_func.index++) 
        stack_push(&args_varlist.list, {par -> value.var, string_hash(par -> value.var), 2 + new_func.index});

    stack_push(&func_list, new_func);

    ASSERT(node -> right, "Function has no sequence!\n");
    read_sequence(node -> right, file, &local_varlist);

    free_varlist(&local_varlist);
    free_varlist(&args_varlist);

    SKIP_LINE();
}


void add_expression(const Node *node, FILE *file, VarList *var_list) {
    switch(node -> type) {
        case TYPE_NUM: {
            PRINTL("push %d", (int)(node -> value.dbl * 1000));
            break;
        }
        case TYPE_VAR: {
            int is_global = 0;
            Variable *var = find_variable(string_hash(node -> value.var), var_list, &is_global);

            ASSERT(var, "Variable %s is not declarated in the current scope!\n", node -> value.var);

            if (is_global)  PRINTL("push QWORD [%s]", var -> name);
            else            PRINTL("push QWORD [rbp + (%i)]", var -> index * 8);

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
                    PRINTL("pop rsi");
                    PRINTL("add rdi, rsi");
                    PRINTL("push rdi");
                    break;
                }
                case OP_SUB: {
                    PRINTL("pop rsi");
                    PRINTL("pop rdi");
                    PRINTL("sub rdi, rsi");
                    PRINTL("push rdi");
                    break;
                }
                case OP_MUL: {
                    PRINTL("pop rax");
                    PRINTL("pop rdi");

                    PRINTL("imul edi");
                    PRINTL("shl rdx, 32");
                    PRINTL("add rax, rdx");

                    PRINTL("mov rdx, rax");
                    PRINTL("shr rdx, 32");
                    PRINTL("mov edi, 1000");
                    PRINTL("idiv edi");
                    
                    PRINTL("cdqe");
                    PRINTL("push rax");
                    break;
                }
                case OP_DIV: {
                    PRINTL("pop rdi");
                    PRINTL("pop rax");
                    
                    PRINTL("mov rdx, rax");
                    PRINTL("shl rdx, 5");
                    PRINTL("mov rcx, rax");
                    PRINTL("shl rcx, 3");
                    PRINTL("shl rax, 10");
                    PRINTL("sub rax, rdx");
                    PRINTL("add rax, rcx");
                    
                    PRINTL("mov rdx, rax");
                    PRINTL("shr rdx, 32");
                    PRINTL("idiv edi");

                    PRINTL("cdqe");
                    PRINTL("push rax");

                    break;
                }

                case OP_EQ:     print_cond("je", file);  break;
                case OP_NEQ:    print_cond("jne", file); break;
                case OP_GRE:    print_cond("jg", file);  break;
                case OP_LES:    print_cond("jl", file);  break;
                case OP_GEQ:    print_cond("jge", file); break;
                case OP_LEQ:    print_cond("jle", file); break;

                case OP_REF: {
                    ASSERT(node -> right, "No expression after referencing operation!\n");

                    add_expression(node -> right, file, var_list);

                    PRINTL("pop rdi");
                    PRINTL("push [rdi]");

                    break;
                }

                case OP_LOC: {
                    ASSERT(node -> right, "No identificator after locate operation!\n");
                    ASSERT(node -> right -> type == TYPE_VAR, "No identificator after locate operation!\n");

                    int is_global = 0;
                    Variable *var = find_variable(string_hash(node -> right -> value.var), var_list, &is_global);

                    ASSERT(var, "Variable %s is not declarated in the current scope!\n", node -> value.var);

                    if (is_global) {
                        PRINTL("lea rdi, [%s]", var -> name);
                        PRINTL("push rdi");
                    }
                    else {
                        PRINTL("lea rdi, [rbp + (%i)]", var -> index * 8);
                        PRINTL("push rdi");
                    }

                    break;
                }

                default: ASSERT(0, "Unexpected operator %i in expression!\n", node -> value.op);
            }

            break;
        }
        default: ASSERT(0, "Node has type %i and it's not expression type!\n", node -> type);
    }
}


void add_assign(const Node *node, FILE *file, VarList *var_list) {
    ASSERT(node -> type == TYPE_OP && node -> value.op == OP_ASS, "Assign expect op %i, but %i got!\n", OP_ASS, node -> value.op);

    ASSERT(node -> right, "No expression to assign!\n");
    CALL_FUNC(add_expression, node -> right);

    ASSERT(node -> left, "No variable to assign!\n");
    
    if (node -> left -> type == TYPE_VAR) {
        int is_global = 0;
        Variable *var = find_variable(string_hash(node -> left -> value.var), var_list, &is_global);

        ASSERT(var, "Variable %s is not declarated in the current scope!\n", node -> left -> value.var);

        if (is_global)  PRINTL("pop QWORD [%s]", var -> name);
        else            PRINTL("pop QWORD [rbp + (%i)]", var -> index * 8);
    }
    else if (node -> left -> type == TYPE_OP && node -> left -> value.op == OP_REF) {
        ASSERT(node -> left -> right, "No expression after referencing operation!\n");

        add_expression(node -> left -> right, file, var_list);

        PRINTL("pop rdi");
        PRINTL("pop [rdi]");
    }
    else {
        ASSERT(0, "Can't assign value to this node!\n");
    }
}


void add_if(const Node *node, FILE *file, VarList *var_list) {
    ASSERT(node -> type == TYPE_IF, "If expect type %i, but %i got!\n", TYPE_IF, node -> type);

    ASSERT(node -> left, "If has no condition!\n");
    CALL_FUNC(add_expression, node -> left);

    PRINTL("pop rdi");
    PRINTL("test rdi, rdi");
    int cur_line = line;
    PRINTL("je IF_%i_FALSE", cur_line);

    ASSERT(node -> right -> left, "If has no sequence after it!\n");

    VarList new_varlist = init_varlist(var_list);
    read_sequence(node -> right -> left, file, &new_varlist);
    free_varlist(&new_varlist);

    PRINTL("jmp IF_%i_END", cur_line);
    PRINT("IF_%i_FALSE:", cur_line);

    if (node -> right -> right) {
        new_varlist = init_varlist(var_list);
        read_sequence(node -> right -> right, file, &new_varlist);
        free_varlist(&new_varlist);
    }

    PRINT("IF_%i_END:", cur_line);
}


void add_while(const Node *node, FILE *file, VarList *var_list) {
    ASSERT(node -> type == TYPE_WHILE, "While expect type %i, but %i got!\n", TYPE_WHILE, node -> type);

    int cur_line = line;

    PRINTL("jmp CYCLE_%i_CONDITION", cur_line);
    PRINTL("CYCLE_%i:", cur_line);

    ASSERT(node -> right, "While has no sequence after it!\n");

    VarList new_varlist = init_varlist(var_list);
    read_sequence(node -> right, file, &new_varlist);
    free_varlist(&new_varlist);

    PRINTL("CYCLE_%i_CONDITION:", cur_line);

    ASSERT(node -> left, "While has no condition!\n");
    CALL_FUNC(add_expression, node -> left);

    PRINTL("pop rdi");
    PRINTL("test rdi, rdi");
    PRINTL("jne CYCLE_%i:", cur_line);
}


void add_function_call(const Node *node, FILE *file, VarList *var_list) {
    ASSERT(node -> type == TYPE_CALL, "Function call expect type %i, but %i got!\n", TYPE_CALL, node -> type);

    Function *func = find_function(string_hash(node -> value.var));

    ASSERT(func, "Function %s was not declarated in the current scope!\n", node -> value.var);

    int arg_count = 0;

    for (const Node *arg = node -> left; arg; arg = arg -> right, arg_count++) {
        ASSERT(arg -> type == TYPE_ARG, "Node is not argument type!\n");

        CALL_FUNC(add_expression, arg -> left);
    }

    ASSERT(arg_count == func -> index, "Function %s expects %i arguments, but got %i!\n", func -> name, func -> index, arg_count);

    PRINTL("call FUNC_%s", node -> value.var + 4);

    PRINTL("add rsp, %d", func -> index * 8);

    PRINTL("push rax");
}


void add_return(const Node *node, FILE *file, VarList *var_list) {
    ASSERT(node -> type == TYPE_RET, "Return expect type %i, but %i got!\n", TYPE_RET, node -> type);

    ASSERT(node -> left, "Return has no expression!\n");
    CALL_FUNC(add_expression, node -> left);
    PRINTL("pop rax");

    PRINTL("mov rsp, rbp");
    PRINTL("pop rbp");
    PRINTL("ret");
}


Variable *find_variable(size_t hash, const VarList *var_list, int *is_global, int max_depth) {
    for(int d = 1; d <= max_depth && var_list; d++, var_list = var_list -> prev)
        for(int i = 0; i < var_list -> list.size; i++)
            if ((var_list -> list.data)[i].hash == hash) {
                if (is_global) *is_global = (var_list -> prev == nullptr);

                return var_list -> list.data + i;
            }

    return nullptr;
}


Function *find_function(size_t hash) {
    for (int i = 0; i < func_list.size; i++)
        if (func_list.data[i].hash == hash) return func_list.data + i;
    
    return nullptr;
}


size_t string_hash(const char *str) {
    return gnu_hash(str, strlen(str) * sizeof(char));
}


size_t gnu_hash(const void *ptr, size_t size) {
    if (!ptr || !size) return 0;
    
    size_t hash = 5381;

    for(size_t i = 0; i < size; i++)
        hash = hash * 33 + ((const char *)(ptr))[i];
    
    return hash;
}


void print_cond(const char *cond_op, FILE *file) {
    PRINTL("pop rax");
    PRINTL("pop rbx");
    PRINTL("cmp rbx, rax");
    PRINTL("mov rdi, 1000");
    PRINTL("%s COND_%i", cond_op, line);
    PRINTL("xor rdi, rdi");
    PRINT("COND_%i:", line - 2);
    PRINTL("push rdi");
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
