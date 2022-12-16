#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libs/tree.hpp"
#include "libs/stack.hpp"
#include "program.hpp"


const int TAB_SIZE = 4;

const char *MAIN_FUNC_NAME = "FUNC_VAR_22B14C_01B8923B";


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
        abort();                        \
    }                                   \
} while (0)

/// Prints with the current offset
#define PRINT(...)                      \
do {                                    \
    fprintf(file, "%*s", shift, "");    \
    fprintf(file, __VA_ARGS__);         \
    fputc('\n', file);                  \
    line++;                             \
} while(0)

/// Prints with the current offset plus one more TAB_SIZE
#define PRINTL(...)                                 \
do {                                                \
    fprintf(file, "%*s", shift + TAB_SIZE, "");     \
    fprintf(file, __VA_ARGS__);                     \
    fputc('\n', file);                              \
    line++;                                         \
} while(0)

/// Prints skip one line to file
#define SKIP_LINE(...) do {                         \
    fputc('\n', file);                              \
    line++;                                         \
} while(0)

/// Contructs new stack of local variables, gives it to sequence, then free it
#define READ_SEQ(varlist, prevlist, node)                  \
VarList varlist = {};                                      \
var_list_constructor(&varlist, prevlist);                  \
read_sequence(node, file, &varlist, shift + TAB_SIZE);     \
var_list_destructor(&varlist)

/// Declarates function for assembler source code output with the same set of parameters
#define DEFINE_FUNC(func_name) void func_name(const Node *node, FILE *file, VarList *var_list, int shift)

/// Calls function for assembler source code output with only argument
#define CALL_FUNC(func_name, node_arg) func_name(node_arg, file, var_list, shift + TAB_SIZE)




/// Current line index in file
int line = 1;

/// List of the global variables
Stack global_list = {};

/// List of the functions
Stack func_list = {};

/// List of ram indexes
Stack index_list = {};


/// Reads definition sequence type node and prints result to file
DEFINE_FUNC(read_def_sequence);

/// Reads sequence type node and prints result to file
DEFINE_FUNC(read_sequence);

/// Add new variable to variable list of the current scope
DEFINE_FUNC(set_new_var);

/// Add new function to function list of the current scope
DEFINE_FUNC(set_new_func);

/// Prints expression to file
DEFINE_FUNC(print_exp);

/// Prints variable assign operation to file
DEFINE_FUNC(print_assign);

/// Prints if operator to file
DEFINE_FUNC(print_if);

/// Prints while operator to file
DEFINE_FUNC(print_while);

/// Prints return statement to file
DEFINE_FUNC(print_return);


/**
 * \brief Finds variable in list by its name hash
 * \param [in] hash Var name hash
 * \param [in] var_list List of variables
 * \param [in] Max depth of recursive search in var list previous
 * \return Pointer to variable or null
*/
Variable *find_variable(size_t hash, const VarList *var_list, int max_depth = INT_MAX);


/**
 * \brief Finds function by its name hash
 * \param [in] hash Function name hash
 * \return Pointer to function
*/
Function *find_function(size_t hash);


/// Calculates hash sum of the string
size_t string_hash(const char *str);

/// Prints condition result to file
void print_cond(const char *cond_op, FILE *file, int shift);

/// Initializes var list stack
void var_list_constructor(VarList *var_list, VarList *prev);

/// Destructs var list stack and set new index
void var_list_destructor(VarList *var_list);




int print_program(const Tree *tree, const char *filename) {
    FILE *file = fopen(filename, "w");

    line = 1;

    int shift = -4;              // Это по факту костыль, чтоб макросы работали без исключений

    stack_constructor(&index_list, 2);
    stack_push(&index_list, {nullptr, 0, 0});
    stack_constructor(&global_list, 2);
    stack_constructor(&func_list, 2);

    VarList var_list = {global_list, nullptr};

    PRINTL("JMP START:");

    read_def_sequence(tree -> root, file, &var_list, shift + TAB_SIZE);

    stack_destructor(&global_list);
    stack_destructor(&func_list);
    stack_destructor(&index_list);

    PRINTL("START:");
    PRINTL("PUSH %i", global_list.size);
    PRINTL("POP RDX");
    PRINTL("CALL %s:", MAIN_FUNC_NAME);

    PRINTL("HLT");

    fclose(file);

    return 0;
}


DEFINE_FUNC(read_def_sequence) {
    for (const Node *iter = node; iter; iter = iter -> right){
        ASSERT(iter, "Definition sequence is null!");

        ASSERT(iter -> type == TYPE_DEF_SEQ, "Definition sequence expect type %i, but %i got!", TYPE_DEF_SEQ, iter -> type);

        ASSERT(iter -> left, "Definition sequence has no left child!");

        PRINT("# Definition sequence node [%-p]", iter);

        switch (iter -> left -> type) {
            case TYPE_NVAR:     CALL_FUNC(set_new_var, iter -> left);       break;
            case TYPE_DEF:      CALL_FUNC(set_new_func, iter -> left);      break;
            default: ASSERT(0, "Definition sequence left child has type %i!", iter -> left -> type);
        }

        SKIP_LINE();
    }
}


DEFINE_FUNC(read_sequence) {
    for (const Node *iter = node; iter; iter = iter -> right){
        ASSERT(iter, "Sequence is null!");

        ASSERT(iter -> type == TYPE_SEQ, "Sequence expect type %i, but %i got!", TYPE_SEQ, iter -> type);

        ASSERT(iter -> left, "Sequence has no left child!");

        PRINT("# Sequence node [%-p]", iter);

        switch (iter -> left -> type) {
            case TYPE_NVAR:     CALL_FUNC(set_new_var, iter -> left);       break;
            case TYPE_OP:       CALL_FUNC(print_assign, iter -> left);      break;
            case TYPE_IF:       CALL_FUNC(print_if, iter -> left);          break;
            case TYPE_WHILE:    CALL_FUNC(print_while, iter -> left);       break;
            case TYPE_RET:      CALL_FUNC(print_return, iter -> left);      break;
            default: ASSERT(0, "Sequence left child has type %i!", iter -> left -> type);
        }

        SKIP_LINE();
    }
}


DEFINE_FUNC(set_new_var) {
    size_t hash = string_hash(node -> value.var);

    ASSERT(!find_variable(hash, var_list, 1), "Variable %s has already been declarated!", node -> value.var);

    stack_push(&var_list -> list, {node -> value.var, hash, index_list.data[index_list.size - 1].index++});

    PRINT("# Add variable %s!", node -> value.var);
}


void print_params(const Node *node, FILE *file, int shift, int index_offset) {
    if (!node) return;

    if (node -> right) print_params(node -> right, file, shift, index_offset + 1);

    PRINT("# Function parameter %s", node -> value.var);
    PRINTL("POP [%i + RDX]", index_offset);
    SKIP_LINE();
}


DEFINE_FUNC(set_new_func) {
    ASSERT(node -> type == TYPE_DEF, "Node is not def type!");
    
    Function new_func = {node -> value.var, string_hash(node -> value.var), 0};

    PRINT("# Function declaration [%-p]", node);
    SKIP_LINE();

    VarList new_varlist = {};
    var_list_constructor(&new_varlist, var_list);

    PRINTL("FUNC_%s:", node -> value.var);
    SKIP_LINE();

    for (const Node *par = node -> left; par; par = par -> right, new_func.index++) 
        stack_push(&new_varlist.list, {par -> value.var, string_hash(par -> value.var), new_func.index});

    print_params(node -> left, file, shift + TAB_SIZE, 0);

    stack_push(&func_list, new_func);

    stack_push(&index_list, {nullptr, 0, new_func.index});

    ASSERT(node -> right, "Function has no sequence!");
    read_sequence(node -> right, file, &new_varlist, shift + TAB_SIZE);

    var_list_destructor(&new_varlist);
}


DEFINE_FUNC(print_exp) {
    switch(node -> type) {
        case TYPE_NUM: {
            PRINT("PUSH %.3lg", node -> value.dbl);
            break;
        }
        case TYPE_VAR: {
            Variable *var = find_variable(string_hash(node -> value.var), var_list);

            ASSERT(var, "Variable %s is not declarated in the current scope!", node -> value.var);

            PRINT("PUSH [%i + RDX]", var -> index);
            break;
        }
        case TYPE_CALL: {
            PRINT("# Call function node [%-p]", node);

            Function *func = find_function(string_hash(node -> value.var));

            shift += 4;

            int arg_count = 0;

            for (const Node *arg = node -> left; arg; arg = arg -> right, arg_count++) {
                PRINT("# Argument node [%-p]", node);

                CALL_FUNC(print_exp, arg -> left);

                SKIP_LINE();
            }

            ASSERT(arg_count == func -> index, "Function %s expects %i arguments, but got %i!", func -> name, func -> index, arg_count);

            PRINT("PUSH RDX");
            PRINT("PUSH %i", index_list.data[index_list.size - 1].index); 
            PRINT("ADD");
            PRINT("POP RDX");
            SKIP_LINE();

            PRINT("CALL FUNC_%s", node -> value.var);

            SKIP_LINE();
            PRINT("PUSH RDX");
            PRINT("PUSH %i", index_list.data[index_list.size - 1].index); 
            PRINT("SUB");
            PRINT("POP RDX");
            SKIP_LINE();

            stack_push(&index_list, {nullptr, 0, 0});

            break;
        }
        case TYPE_OP: {
            PRINT("# Expression node [%-p]", node);

            CALL_FUNC(print_exp, node -> left);
            CALL_FUNC(print_exp, node -> right);

            shift += 4;

            switch (node -> value.op) {
                case OP_ADD: PRINT("ADD"); break;
                case OP_SUB: PRINT("SUB"); break;
                case OP_MUL: PRINT("MUL"); break;
                case OP_DIV: PRINT("DIV"); break;

                case OP_EQ:     print_cond("JE", file, shift);  break;
                case OP_NEQ:    print_cond("JNE", file, shift); break;
                case OP_GRE:    print_cond("JA", file, shift);  break;
                case OP_LES:    print_cond("JB", file, shift);  break;
                case OP_GEQ:    print_cond("JAE", file, shift); break;
                case OP_LEQ:    print_cond("JBE", file, shift); break;

                default: ASSERT(0, "Unexpected operator %i in expression!", node -> value.op);
            }

            break;
        }
        default: ASSERT(0, "Node has type %i and it's not expression type!", node -> type);
    }
}


DEFINE_FUNC(print_assign) {
    PRINT("# Assign node [%-p]", node);

    ASSERT(node -> type == TYPE_OP && node -> value.op == OP_ASS, "Assign expect op %i, but %i got!", OP_ASS, node -> value.op);

    ASSERT(node -> right, "No expression to assign!");
    CALL_FUNC(print_exp, node -> right);

    ASSERT(node -> left, "No variable to assign!");
    Variable *var = find_variable(string_hash(node -> left -> value.var), var_list);

    ASSERT(var, "Variable %s is not declarated in the current scope!", node -> left -> value.var);

    PRINTL("POP [%i + RDX]", var -> index);
}


DEFINE_FUNC(print_if) {
    ASSERT(node -> type == TYPE_IF, "If expect type %i, but %i got!", TYPE_IF, node -> type);

    PRINT("# If node [%-p]", node);

    ASSERT(node -> left, "If has no condition!");
    CALL_FUNC(print_exp, node -> left);

    PRINTL("PUSH 0");
    int cur_line = line;
    PRINTL("JE IF_%i_FALSE", cur_line);

    SKIP_LINE();

    ASSERT(node -> right, "If has no sequence!");

    READ_SEQ(new_var_list, var_list, node -> right);

    PRINTL("IF_%i_FALSE:", cur_line);
}


DEFINE_FUNC(print_while) {
    ASSERT(node -> type == TYPE_WHILE, "While expect type %i, but %i got!", TYPE_WHILE, node -> type);

    PRINT("# While node [%-p]", node);

    int cur_line = line;

    PRINTL("CYCLE_%i_ITER:", cur_line);

    ASSERT(node -> left, "If has no condition!");
    CALL_FUNC(print_exp, node -> left);

    PRINTL("PUSH 0");
    PRINTL("JE CYCLE_%i_FALSE", cur_line);

    SKIP_LINE();

    ASSERT(node -> right, "If has no sequence!");

    READ_SEQ(new_var_list, var_list, node -> right);

    PRINTL("JMP CYCLE_%i_ITER", cur_line);

    PRINTL("CYCLE_%i_FALSE:", cur_line);
}


DEFINE_FUNC(print_return) {
    ASSERT(node -> type == TYPE_RET, "Return expect type %i, but %i got!", TYPE_RET, node -> type);

    PRINT("# Return node [%-p]", node);

    ASSERT(node -> left, "Return has no expression!");
    CALL_FUNC(print_exp, node -> left);

    stack_data_t tmp = {};
    stack_pop(&index_list, &tmp);

    PRINTL("RET");
}


Variable *find_variable(size_t hash, const VarList *var_list, int max_depth) {
    for(int d = 1; d <= max_depth && var_list; d++, var_list = var_list -> prev)
        for(int i = 0; i < var_list -> list.size; i++)
            if ((var_list -> list.data)[i].hash == hash) return var_list -> list.data + i;

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


void print_cond(const char *cond_op, FILE *file, int shift) {
    PRINT("# Condition");
    PRINT("PUSH 1");
    PRINT("POP RAX");
    PRINT("%s COND_%i", cond_op, line);
    PRINT("PUSH 0");
    PRINT("POP RAX");
    PRINT("COND_%i:", line - 3);
    PRINT("PUSH RAX");

    SKIP_LINE();
}


void var_list_constructor(VarList *var_list, VarList *prev) {
    stack_constructor(&var_list -> list, 2);

    var_list -> prev = prev;
}


void var_list_destructor(VarList *var_list) {
    index_list.data[index_list.size - 1].index -= (var_list -> list).size;

    stack_destructor(&var_list -> list);

    var_list -> prev = nullptr;
}
