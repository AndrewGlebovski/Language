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

/// Add new variable to variable list of the current scope
void add_variable(const Node *node, FILE *file, VarList *var_list);

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

/// Prints function parameters in reverse order
void add_parameters(const Node *node, FILE *file, int index_offset);


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
int count_variables(const VarList *varlist);


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
        {"VAR_22B14C_0013A52700E2108E01151151", string_hash("VAR_22B14C_0013A52700E2108E01151151"), 3},
        {"VAR_22B14C_0194AD2B", string_hash("VAR_22B14C_0194AD2B"), 0},
    };

    for (int i = 0; i < (int)(sizeof(lib) / sizeof(Function)); i++) stack_push(&func_list, lib[i]);

    PRINT("JMP START:");
    SKIP_LINE();

    include_file("stdlib.asm", file);

    PRINT("# Source code");

    read_def_sequence(tree -> root, file, &global_list);

    if (!find_function(string_hash(MAIN_FUNC))) {
        printf("Main function was not declarated in the current scope!\n");
        abort();
    }

    SKIP_LINE();
    PRINT("START:");
    PRINT("PUSH %i", global_list.list.size);
    PRINT("POP RDX");
    PRINT("CALL FUNC_%s:", MAIN_FUNC);

    free_varlist(&global_list);

    stack_destructor(&func_list);

    PRINT("HLT");

    fclose(file);

    return 0;
}


void include_file(const char *filename, FILE *file) {
    int origin = open(filename, O_RDONLY);

    Text text = {};
    read_text(&text, origin);

    PRINT("# Included from %s", filename);
    
    for (String *i = text.lines; i -> str; i++) PRINT("%s", i -> str);

    close(origin);
}


void read_def_sequence(const Node *node, FILE *file, VarList *var_list) {
    for (const Node *iter = node; iter; iter = iter -> right){
        ASSERT(iter, "Definition sequence is null!\n");

        ASSERT(iter -> type == TYPE_DEF_SEQ, "Definition sequence expect type %i, but %i got!\n", TYPE_DEF_SEQ, iter -> type);

        ASSERT(iter -> left, "Definition sequence has no left child!\n");

        switch (iter -> left -> type) {
            case TYPE_NVAR:     CALL_FUNC(add_variable, iter -> left);       break;
            case TYPE_DEF:      CALL_FUNC(add_function, iter -> left);      break;
            default: ASSERT(0, "Definition sequence left child has type %i!\n", iter -> left -> type);
        }
    }
}


void read_sequence(const Node *node, FILE *file, VarList *var_list) {
    for (const Node *iter = node; iter; iter = iter -> right){
        ASSERT(iter, "Sequence is null!\n");

        ASSERT(iter -> type == TYPE_SEQ, "Sequence expect type %i, but %i got!\n", TYPE_SEQ, iter -> type);

        ASSERT(iter -> left, "Sequence has no left child!\n");

        switch (iter -> left -> type) {
            case TYPE_NVAR:     CALL_FUNC(add_variable, iter -> left);                               break;
            case TYPE_OP:       CALL_FUNC(add_assign, iter -> left);                              break;
            case TYPE_IF:       CALL_FUNC(add_if, iter -> left);                                  break;
            case TYPE_WHILE:    CALL_FUNC(add_while, iter -> left);                               break;
            case TYPE_RET:      CALL_FUNC(add_return, iter -> left);                              break;
            case TYPE_CALL:     CALL_FUNC(add_function_call, iter -> left); PRINTL("POP RBX");    break;
            default: ASSERT(0, "Sequence left child has type %i!\n", iter -> left -> type);
        }

        
    }
}


void add_variable(const Node *node, FILE *file, VarList *var_list) {
    ASSERT(node -> type == TYPE_NVAR, "Node is not new variable type!\n");

    size_t hash = string_hash(node -> value.var);

    int is_global = 0;
    ASSERT(!find_variable(hash, var_list, &is_global, 1), "Variable %s has already been declarated!\n", node -> value.var);

    Variable new_var = {node -> value.var, hash, count_variables(var_list)};

    stack_push(&var_list -> list, new_var);

    ASSERT(node -> right, "New variable has no expression to assign!\n");
    CALL_FUNC(add_expression, node -> right);

    PRINTL("POP [%i%s]", new_var.index, (is_global)? "" : " + RDX");
}


void add_parameters(const Node *node, FILE *file, int index_offset) {
    if (!node) return;

    ASSERT(node -> type == TYPE_PAR, "Node is not parameter type!\n");

    if (node -> right) add_parameters(node -> right, file, index_offset + 1);

    PRINTL("POP [%i + RDX]", index_offset);
}


void add_function(const Node *node, FILE *file, VarList *var_list) {
    ASSERT(node -> type == TYPE_DEF, "Node is not function define type!\n");
    
    ASSERT(!find_function(string_hash(node -> value.var)), "Function %s has already been declarated!\n", node -> value.var);

    Function new_func = {node -> value.var, string_hash(node -> value.var), 0};

    VarList new_varlist = init_varlist(var_list);

    PRINT("FUNC_%s:", node -> value.var);

    for (const Node *par = node -> left; par; par = par -> right, new_func.index++) 
        stack_push(&new_varlist.list, {par -> value.var, string_hash(par -> value.var), new_func.index});

    add_parameters(node -> left, file, 0);

    stack_push(&func_list, new_func);

    ASSERT(node -> right, "Function has no sequence!\n");
    read_sequence(node -> right, file, &new_varlist);

    free_varlist(&new_varlist);

    SKIP_LINE();
}


void add_expression(const Node *node, FILE *file, VarList *var_list) {
    switch(node -> type) {
        case TYPE_NUM: {
            PRINTL("PUSH %.3f", node -> value.dbl);
            break;
        }
        case TYPE_VAR: {
            int is_global = 0;
            Variable *var = find_variable(string_hash(node -> value.var), var_list, &is_global);

            ASSERT(var, "Variable %s is not declarated in the current scope!\n", node -> value.var);

            PRINTL("PUSH [%i%s]", var -> index, (is_global)? "" : " + RDX");
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
                case OP_ADD: PRINTL("ADD"); break;
                case OP_SUB: PRINTL("SUB"); break;
                case OP_MUL: PRINTL("MUL"); break;
                case OP_DIV: PRINTL("DIV"); break;

                case OP_EQ:     print_cond("JE", file);  break;
                case OP_NEQ:    print_cond("JNE", file); break;
                case OP_GRE:    print_cond("JA", file);  break;
                case OP_LES:    print_cond("JB", file);  break;
                case OP_GEQ:    print_cond("JAE", file); break;
                case OP_LEQ:    print_cond("JBE", file); break;

                case OP_REF: {
                    ASSERT(node -> right, "No expression after referencing operation!\n");

                    add_expression(node -> right, file, var_list);

                    PRINTL("POP RAX");
                    PRINTL("PUSH [RAX]");

                    break;
                }

                case OP_LOC: {
                    ASSERT(node -> right, "No identificator after locate operation!\n");
                    ASSERT(node -> right -> type == TYPE_VAR, "No identificator after locate operation!\n");

                    int is_global = 0;
                    Variable *var = find_variable(string_hash(node -> right -> value.var), var_list, &is_global);

                    ASSERT(var, "Variable %s is not declarated in the current scope!\n", node -> value.var);

                    PRINTL("PUSH %i%s", var -> index, (is_global)? "" : " + RDX");

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

        PRINTL("POP [%i%s]", var -> index, (is_global)? "" : " + RDX");
    }
    else if (node -> left -> type == TYPE_OP && node -> left -> value.op == OP_REF) {
        ASSERT(node -> left -> right, "No expression after referencing operation!\n");

        add_expression(node -> left -> right, file, var_list);

        PRINTL("POP RAX");
        PRINTL("POP [RAX]");
    }
    else {
        ASSERT(0, "Can't assign value to this node!\n");
    }
}


void add_if(const Node *node, FILE *file, VarList *var_list) {
    ASSERT(node -> type == TYPE_IF, "If expect type %i, but %i got!\n", TYPE_IF, node -> type);

    ASSERT(node -> left, "If has no condition!\n");
    CALL_FUNC(add_expression, node -> left);

    PRINTL("PUSH 0");
    int cur_line = line;
    PRINTL("JE IF_%i_FALSE", cur_line);

    ASSERT(node -> right -> left, "If has no sequence after it!\n");

    VarList new_varlist = init_varlist(var_list);
    read_sequence(node -> right -> left, file, &new_varlist);
    free_varlist(&new_varlist);

    PRINTL("JMP IF_%i_END", cur_line);
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

    PRINT("CYCLE_%i_ITER:", cur_line);

    ASSERT(node -> left, "While has no condition!\n");
    CALL_FUNC(add_expression, node -> left);

    PRINTL("PUSH 0");
    PRINTL("JE CYCLE_%i_FALSE", cur_line);

    ASSERT(node -> right, "While has no sequence after it!\n");

    VarList new_varlist = init_varlist(var_list);
    read_sequence(node -> right, file, &new_varlist);
    free_varlist(&new_varlist);

    PRINTL("JMP CYCLE_%i_ITER", cur_line);

    PRINT("CYCLE_%i_FALSE:", cur_line);
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

    PRINTL("PUSH RDX");
    PRINTL("PUSH %i", count_variables(var_list)); 
    PRINTL("ADD");
    PRINTL("POP RDX");

    PRINTL("CALL FUNC_%s", node -> value.var);

    PRINTL("PUSH RDX");
    PRINTL("PUSH %i", count_variables(var_list)); 
    PRINTL("SUB");
    PRINTL("POP RDX");
    
}


void add_return(const Node *node, FILE *file, VarList *var_list) {
    ASSERT(node -> type == TYPE_RET, "Return expect type %i, but %i got!\n", TYPE_RET, node -> type);

    ASSERT(node -> left, "Return has no expression!\n");
    CALL_FUNC(add_expression, node -> left);

    PRINTL("RET");
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
    PRINTL("PUSH 1");
    PRINTL("POP RAX");
    PRINTL("%s COND_%i", cond_op, line);
    PRINTL("PUSH 0");
    PRINTL("POP RAX");
    PRINT("COND_%i:", line - 3);
    PRINTL("PUSH RAX");
}


VarList init_varlist(VarList *prev) {
    VarList varlist = {{}, prev};
    
    stack_constructor(&varlist.list, 2);
    
    return varlist;
}


void free_varlist(VarList *varlist) {
    stack_destructor(&varlist -> list);
}


int count_variables(const VarList *varlist) {
    int count = 0;

    for(const VarList *iter = varlist; iter -> prev; iter = iter -> prev)
        count += iter -> list.size;

    return count;
}
