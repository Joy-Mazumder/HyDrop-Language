// ast.h
#ifndef AST_H
#define AST_H

typedef enum {

    NODETYPE_NUMBER, NODETYPE_VAR, NODETYPE_ASSIGN,
    NODETYPE_ADD, NODETYPE_SUB, NODETYPE_MUL, NODETYPE_DIV,
    NODETYPE_GT, NODETYPE_LT, NODETYPE_EQ, NODETYPE_NEQ, NODETYPE_GTE, NODETYPE_LTE,
    NODETYPE_IF, NODETYPE_WHILE, NODETYPE_PRINT, NODETYPE_INPUT,
    NODETYPE_STRING,
    // Phase 3-7 (Advanced Master Plan Features)
    NODETYPE_LET_ONCE, NODETYPE_HISTORY, NODETYPE_BENCHMARK,
    NODETYPE_REACTIVE, NODETYPE_RETRY, NODETYPE_BLOCK, NODETYPE_SAFE_EXTRACT,
    NODETYPE_DRAW_CHART,
    NODETYPE_DUMP_MEMORY

} NodeType;


#define ASSIGN_NORMAL   0
#define ASSIGN_REACTIVE 1
#define ASSIGN_QUANTUM  2
#define ASSIGN_MONEY    3

// Tree er structure (AST Node)
typedef struct ASTNode {
    NodeType type;

    
    struct ASTNode *left;
    struct ASTNode *right;
    struct ASTNode *condition;
    struct ASTNode *body;

    
    float float_val;
    char *var_name;
    char *str_val;
} ASTNode;


extern int explain_mode;


ASTNode* new_num_node(float val);
ASTNode* new_var_node(char* name);
ASTNode* new_op_node(NodeType type, ASTNode* left, ASTNode* right);
ASTNode* new_assign_node(char* name, ASTNode* expr, int mode);
ASTNode* new_if_node(ASTNode* cond, ASTNode* if_body, ASTNode* else_body);
ASTNode* new_while_node(ASTNode* cond, ASTNode* body);
ASTNode* new_print_node(ASTNode* expr);
ASTNode* new_special_node(NodeType type, ASTNode* left, ASTNode* right); // History, SafeExtract, Chart
ASTNode* new_retry_node(ASTNode* body, ASTNode* fallback, int count);    // Phase 5: Auto-Retry
ASTNode* new_string_node(char* val);
ASTNode* new_input_node(char* name);

void execute_ast(ASTNode* node); 

#endif