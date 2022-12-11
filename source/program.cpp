#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libs/tree.hpp"
#include "libs/stack.hpp"
#include "program.hpp"


typedef stack_data_t Variable;



#define ASSERT(condition, ...)          \
do                                      \
{                                       \
    if (!(condition)) {                 \
        printf("[%-p] ", node);         \
        printf(__VA_ARGS__);            \
        abort();                        \
    }                                   \
} while (0)


#define PRINT(...)                      \
do {                                    \
    fprintf(file, __VA_ARGS__);         \
    fputc('\n', file);                  \
    line++;                             \
} while(0)


/// List of variables in program
Variable *vars = nullptr;

/// Current line index in file
int line = 1;


/// Reads sequence type node and prints result to file
void read_sequence(const Node *node, FILE *file);

/// Add new variable to vars
void set_new_var(const Node *node, FILE *file);

/// Prints expression to file
void print_exp(const Node *node, FILE *file);

/// Prints variable assign operation to file
void print_assign(const Node *node, FILE *file);

/// Prints if operator to file
void print_if(const Node *node, FILE *file);

/// Prints while operator to file
void print_while(const Node *node, FILE *file);

/**
 * \brief Finds variable in list by its name hash
 * \param [in] hash Var name hash
 * \return Pointer to variable or null
*/
Variable *find_variable(size_t hash);

/// Calculates hash sum of the string
size_t string_hash(const char *str);

/// Prints condition result to file
void print_cond(const char *cond_op, FILE *file);




int print_program(const Tree *tree, const char *filename) {
    FILE *file = fopen(filename, "w");

    vars = (Variable *) calloc(64, sizeof(Variable));
    line = 1;

    read_sequence(tree -> root, file);

    free(vars);

    PRINT("HLT");

    fclose(file);

    return 0;
}


void read_sequence(const Node *node, FILE *file) {
    ASSERT(node, "Sequence is null!");

    ASSERT(node -> type == TYPE_SEQ, "Sequence expect type %i, but %i got!", TYPE_SEQ, node -> type);

    ASSERT(node -> left, "Sequence has no left child!");

    PRINT("# Sequence node [%-p]", node);

    switch (node -> left -> type) {
        case TYPE_NVAR: set_new_var(node -> left, file); break;
        case TYPE_OP: print_assign(node -> left, file); break;
        case TYPE_IF: print_if(node -> left, file); break;
        case TYPE_WHILE: print_while(node -> left, file); break;
        default: ASSERT(0, "Sequence left child has type %i!", node -> left -> type);
    }

    PRINT(" ");
    PRINT(" ");

    if (node -> right) read_sequence(node -> right, file);
}


void set_new_var(const Node *node, FILE* file) {
    size_t hash = string_hash(node -> value.var);

    ASSERT(!find_variable(hash), "Variable %s has already been declarated!", node -> value.var);

    Variable *new_var = nullptr;

    for(new_var = vars; new_var -> hash; new_var++); // Find empty variable

    *new_var = {node -> value.var, hash, (int)(vars - new_var)};

    PRINT("# Add variable %s!", node -> value.var);
}


void print_exp(const Node *node, FILE *file) {
    switch(node -> type) {
        case TYPE_NUM: {
            PRINT("PUSH %.3lg", node -> value.dbl);
            break;
        }
        case TYPE_VAR: {
            Variable *var = find_variable(string_hash(node -> value.var));

            ASSERT(var, "Variable %s is not declarated in the current scope!", node -> value.var);

            PRINT("PUSH [%i]", var -> index);
            break;
        }
        case TYPE_OP: {
            print_exp(node -> left, file);
            print_exp(node -> right, file);

            PRINT("# Expression node [%-p]", node);
            switch (node -> value.op) {
                case OP_ADD: PRINT("ADD"); break;
                case OP_SUB: PRINT("SUB"); break;
                case OP_MUL: PRINT("MUL"); break;
                case OP_DIV: PRINT("DIV"); break;

                case OP_EQ: print_cond("JE", file); break;
                case OP_NEQ: print_cond("JNE", file); break;
                case OP_GRE: print_cond("JA", file); break;
                case OP_LES: print_cond("JB", file); break;
                case OP_GEQ: print_cond("JAE", file); break;
                case OP_LEQ: print_cond("JBE", file); break;

                default: ASSERT(0, "Unexpected operator %i in expression!", node -> value.op);
            }

            PRINT(" ");

            break;
        }
        default: ASSERT(0, "Node has type %i and it's not expression type!", node -> type);
    }
}


void print_assign(const Node *node, FILE *file) {
    ASSERT(node -> type == TYPE_OP && node -> value.op == OP_ASS, "Assign expect op %i, but %i got!", OP_ASS, node -> value.op);

    ASSERT(node -> right, "No expression to assign!");
    print_exp(node -> right, file);

    ASSERT(node -> left, "No variable to assign!");
    Variable *var = find_variable(string_hash(node -> left -> value.var));

    ASSERT(var, "Variable %s is not declarated in the current scope!", node -> left -> value.var);

    PRINT("# Assign node [%-p]", node);
    PRINT("POP [%i]", var -> index);
}


void print_if(const Node *node, FILE *file) {
    ASSERT(node -> type == TYPE_IF, "If expect type %i, but %i got!", TYPE_IF, node -> type);

    ASSERT(node -> left, "If has no condition!");
    print_exp(node -> left, file);

    PRINT("PUSH 0");
    int cur_line = line;
    PRINT("JE IF_%i_FALSE", cur_line);

    ASSERT(node -> right, "If has no sequence!");
    read_sequence(node -> right, file);

    PRINT("IF_%i_FALSE:", cur_line);
}


void print_while(const Node *node, FILE *file) {
    ASSERT(node -> type == TYPE_WHILE, "While expect type %i, but %i got!", TYPE_WHILE, node -> type);

    int cur_line = line;

    PRINT("CYCLE_%i_ITER:", cur_line);

    ASSERT(node -> left, "If has no condition!");
    print_exp(node -> left, file);

    PRINT("PUSH 0");
    PRINT("JE CYCLE_%i_FALSE", cur_line);

    ASSERT(node -> right, "If has no sequence!");
    read_sequence(node -> right, file);

    PRINT("JMP CYCLE_%i_ITER", cur_line);

    PRINT("CYCLE_%i_FALSE:", cur_line);
}


Variable *find_variable(size_t hash) {
    for(Variable *i = vars; i -> hash; i++)
        if (i -> hash == hash) return i;
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
    PRINT("PUSH 1");
    PRINT("POP RAX");
    PRINT("%s COND_%i", cond_op, line);
    PRINT("PUSH 0");
    PRINT("POP RAX");
    PRINT("COND_%i:", line - 3);
    PRINT("PUSH RAX");
}