/// Possible function exit codes
typedef enum {
    INVALID_ARG  =  1,       ///< Function got invalid argument
    ALLOC_FAIL   =  2,       ///< Failed to allocate memory
} ERROR_CODES;


typedef enum {
    OP_ADD = 1,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_AND,
    OP_OR,
    OP_NOT,
    OP_ASS,
    OP_EQ,
    OP_NEQ,
    OP_GRE,
    OP_LES,
    OP_GEQ,
    OP_LEQ,
    OP_DIF,
} OPERATORS;


typedef enum {
    TYPE_ESC,
    TYPE_OP,
    TYPE_NUM,
    TYPE_VAR,
    TYPE_IF,
    TYPE_WHILE,
    TYPE_CALL,
    TYPE_DEF,
    TYPE_NVAR,
    TYPE_ARG,
    TYPE_PAR,
    TYPE_SEQ,
    TYPE_BLOCK,
    TYPE_BRACKET,
    TYPE_CONT,
    TYPE_RET,
    TYPE_DEF_SEQ,
    TYPE_ELSE,
    TYPE_BRANCH
} NODE_TYPES;


typedef union {
    int    op;
    double dbl;
    char   *var;
} NodeValue; 


/// Node class
struct Node {
    int type = 0;                       ///< Node value
    NodeValue value;                    ///< Node value
    Node *left = nullptr;               ///< Left child node
    Node *right = nullptr;              ///< Right child node
};


/// Tree class
typedef struct {
    Node *root = nullptr;               ///< Tree root node
    int size = 0;                       ///< Nodes count
} Tree;


/**
 * \brief Constructs tree
 * \param [out] tree Previously initialized tree structure
 * \param [in]  root If not null, then it will be set as root of the tree,
 * otherwise root will be allocated
 * \return Non zero value means error
*/
int tree_constructor(Tree *tree, Node *root = nullptr);


/**
 * \brief Allocates new node
 * \param [in] value New node's value
 * \param [in] left  New node's left child
 * \param [in] right New node's right child
 * \return Pointer to node or nullptr if allocates fail
*/
Node *create_node(int type, NodeValue value, Node *left = nullptr, Node *right = nullptr);


/**
 * \brief Destructs tree
 * \param [in] tree All tree nodes will be free
 * \return Non zero value means error
*/
int tree_destructor(Tree *tree);


/**
 * \brief Prints tree
 * \param [in]  node Tree to print
 * \param [out] stream Output stream
*/
void print_tree(Tree *tree, FILE *stream = stdout);


/**
 * \brief Converts operator from #OPERATORS to its string value
 * \param [in] op Operator to convert
 * \return Const string or "#" if symbol is unknown
*/
const char *op2str(int op);


/**
 * \brief Converts symbol to operator from #OPERATORS
 * \param [in] op Operator symbol
 * \return Operator value or OP_ERR if symbol is unknown
*/
int chr2op(char op);


/**
 * \brief Creates enumerated list of dot files and images, also adds information to hmtl file
 * \param [in]  tree Tree to print
 * \return Non zero value means error
*/
int graphic_dump(Tree *tree);


/**
 * \brief Generates text file for dot
 * \param [in]  tree Tree to print
 * \param [out] log  Log file output
 * \return Non zero value means error
*/
int generate_file(Tree *tree, FILE *file);


/**
 * \brief Generates image using dot call
 * \param [in]  input  Path to text file for dot
 * \param [out] output Path to output image
 * \return Non zero value means error
*/
int generate_image(const char *input, const char *output);


/**
 * \brief Pause program so you can watch selected image
 * \param [in]  filepath Path to image file
 * \return Non zero value means error
*/
int show_image(const char *filepath);
