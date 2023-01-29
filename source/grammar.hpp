/* SYNTAX RULES

program ::= {definition} '\0'

definition ::=
    'var' ident = derivative ';'
    | 'def' ident '(' {ident}? {',' ident} ')' statement

statement ::=
    'var' ident = derivative ';'
    | ident '=' derivative ';'
    | '*' factor '=' condition ';'
    | ident '(' {derivative}? {',' derivative} ')' ';'
    | 'begin' {statement ';'} 'end'
    | 'if' '(' condition ')' statement {'else' statement}?
    | 'while' '(' condition ')' statement
    | 'return' derivative ';'

condition ::= derivative {['==''!=''>''<''>=''<='] derivative}?

derivative ::= expression | expression 'd' ident

expression ::= term {['+''-'] term}

term ::= unary {['*''/'] unary}

unary ::= '-' factor | '*' factor | '&' ident | factor

factor ::= '(' condition ')' | function | number

function ::= ident | ident '(' {condition}? {',' condition} ')'

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

Node *get_definition(Node **s);

Node *get_statement(Node **s);

Node *get_condition(Node **s);

Node *get_derivative(Node **s);

Node *get_expression(Node **s);

Node *get_term(Node **s);

Node *get_unary(Node **s);

Node *get_factor(Node **s);

Node *get_function(Node **s);

Node *get_ident(Node **s);

Node *get_number(Node **s);
