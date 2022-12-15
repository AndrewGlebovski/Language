#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libs/tree.hpp"
#include "libs/stack.hpp"
#include "program.hpp"


const int TAB_SIZE = 4;


typedef stack_data_t Variable;


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

/// Index of the first free RAM
int free_ram_index = 0;


/// Reads sequence type node and prints result to file
DEFINE_FUNC(read_sequence);

/// Add new variable to variable list of the current scope
DEFINE_FUNC(set_new_var);

/// Prints expression to file
DEFINE_FUNC(print_exp);

/// Prints variable assign operation to file
DEFINE_FUNC(print_assign);

/// Prints if operator to file
DEFINE_FUNC(print_if);

/// Prints while operator to file
DEFINE_FUNC(print_while);

/**
 * \brief Finds variable in list by its name hash
 * \param [in] hash Var name hash
 * \param [in] var_list List of variables
 * \param [in] Max depth of recursive search in var list previous
 * \return Pointer to variable or null
*/
Variable *find_variable(size_t hash, const VarList *var_list, int max_depth = INT_MAX);

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

    READ_SEQ(var_list, nullptr, tree -> root);

    PRINTL("HLT");

    fclose(file);

    return 0;
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
            default: ASSERT(0, "Sequence left child has type %i!", iter -> left -> type);
        }

        PRINT("%s", ""); // Тоже костыль, чтобы пропустить строку для ясности в ассемблере
        PRINT("%s", "");
    }
}


DEFINE_FUNC(set_new_var) {
    size_t hash = string_hash(node -> value.var);

    ASSERT(!find_variable(hash, var_list, 1), "Variable %s has already been declarated!", node -> value.var);

    stack_push(&var_list -> list, {node -> value.var, hash, free_ram_index++});

    PRINT("# Add variable %s!", node -> value.var);
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

            PRINT("PUSH [%i]", var -> index);
            break;
        }
        case TYPE_OP: {
            PRINT("# Expression node [%-p]", node);

            CALL_FUNC(print_exp, node -> left);
            CALL_FUNC(print_exp, node -> right);

            shift += 4;

            PRINT("%s", "");

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

    PRINT("%s", "");
}


DEFINE_FUNC(print_assign) {
    PRINT("# Assign node [%-p]", node);

    ASSERT(node -> type == TYPE_OP && node -> value.op == OP_ASS, "Assign expect op %i, but %i got!", OP_ASS, node -> value.op);

    ASSERT(node -> right, "No expression to assign!");
    CALL_FUNC(print_exp, node -> right);

    ASSERT(node -> left, "No variable to assign!");
    Variable *var = find_variable(string_hash(node -> left -> value.var), var_list);

    ASSERT(var, "Variable %s is not declarated in the current scope!", node -> left -> value.var);

    PRINTL("POP [%i]", var -> index);
}


DEFINE_FUNC(print_if) {
    ASSERT(node -> type == TYPE_IF, "If expect type %i, but %i got!", TYPE_IF, node -> type);

    PRINT("# If node [%-p]", node);

    ASSERT(node -> left, "If has no condition!");
    CALL_FUNC(print_exp, node -> left);

    PRINTL("PUSH 0");
    int cur_line = line;
    PRINTL("JE IF_%i_FALSE", cur_line);

    PRINT("%s", "");

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

    PRINT("%s", "");

    ASSERT(node -> right, "If has no sequence!");

    READ_SEQ(new_var_list, var_list, node -> right);

    PRINTL("JMP CYCLE_%i_ITER", cur_line);

    PRINTL("CYCLE_%i_FALSE:", cur_line);
}


Variable *find_variable(size_t hash, const VarList *var_list, int max_depth) {
    for(int d = 1; d <= max_depth && var_list; d++, var_list = var_list -> prev)
        for(int i = 0; i < var_list -> list.size; i++)
            if ((var_list -> list.data)[i].hash == hash) return var_list -> list.data + i;

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
    PRINT("PUSH 1");
    PRINT("POP RAX");
    PRINT("%s COND_%i", cond_op, line);
    PRINT("PUSH 0");
    PRINT("POP RAX");
    PRINT("COND_%i:", line - 3);
    PRINT("PUSH RAX");
}


void var_list_constructor(VarList *var_list, VarList *prev) {
    stack_constructor(&var_list -> list, 2);

    var_list -> prev = prev;
}


void var_list_destructor(VarList *var_list) {
    free_ram_index = (var_list -> list).data[0].index; // New free RAM index

    stack_destructor(&var_list -> list);

    var_list -> prev = nullptr;
}
