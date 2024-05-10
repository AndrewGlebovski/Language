// Often used math contants

const double _EXP = 2.71828;
const double _PI = 3.14159;

// Ultimate tools

#define FREE(ptr) free(ptr); ptr = nullptr


/// Compares two doubles
int is_equal(double a, double b);


// Tree creation process

/**
 * \brief Creates number type node with value = num
 * \param [in] num New node value
 * \return New allocated node
*/
Node *create_num(double num);


/**
 * \brief Creates copy of existing node
 * \param [in] node Original node
 * \return New allocated copy
*/
Node *create_copy(const Node *node);


#define Add(left, right) create_node(NODE_TYPES::TYPE_OP, {OPERATORS::OP_ADD}, left, right)
#define Sub(left, right) create_node(NODE_TYPES::TYPE_OP, {OPERATORS::OP_SUB}, left, right)
#define Mul(left, right) create_node(NODE_TYPES::TYPE_OP, {OPERATORS::OP_MUL}, left, right)
#define Div(left, right) create_node(NODE_TYPES::TYPE_OP, {OPERATORS::OP_DIV}, left, right)

// Derivative process

#define L create_copy(node -> left)
#define R create_copy(node -> right)

#define dL diff(node -> left, dif_var)
#define dR diff(node -> right, dif_var)

#define IS_TYPE(_node, _type) _node -> type == TYPE_##_type

// Optimization process

#define IS_OP(_op) node -> type == NODE_TYPES::TYPE_OP && node -> value.op == OP_##_op
#define IS_NUM(_child, _num) (node -> _child -> type == NODE_TYPES::TYPE_NUM && is_equal(node -> _child -> value.dbl, _num))
