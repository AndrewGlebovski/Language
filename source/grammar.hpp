/* SYNTAX RULES

program ::= statement '\0'

statement ::=
    'var' ident ';'
    | 'def' ident '(' {ident}? {',' ident} ')' statement
    | ident '=' expression ';'
    | 'begin' {statement ';'} 'end'
    | 'if' '(' condition ')' statement
    | 'while' '(' condition ')' statement
    | 'return' expression ';'

condition ::= expression {['==''!=''>''<''>=''<='] expression}?

expression ::= term {['+''-'] term}

term ::= unary {['*''/'] unary}

unary ::= '-' factor | factor

factor ::= '(' expression ')' | function | number

function ::= ident | ident '(' {expression}? {',' expression} ')'

ident ::= ['A'-'Z']+

number ::= ['0'-'9']+

*/


/// Allocates copy of the origin node
Node *copy_node(const Node *origin);


/// Recursive call for block content
Node *get_block_value(Node **s);

/// Recursive call for function parameters
Node *get_function_parameters(Node **s);

/// Recursive call for function arguments
Node *get_function_arguments(Node **s);


// Recursive descent parser

Node *get_program(Node *str);

Node *get_statement(Node **s);

Node *get_condition(Node **s);

Node *get_expression(Node **s);

Node *get_term(Node **s);

Node *get_unary(Node **s);

Node *get_factor(Node **s);

Node *get_function(Node **s);

Node *get_ident(Node **s);

Node *get_number(Node **s);
