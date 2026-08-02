%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

int yylex(void);
void yyerror(const char *s);
extern FILE *yyin;


ASTNode* root;
%}

%union {
    float num;
    char* str;
    struct ASTNode* ast;
}

/* Tokens matching lexer.l */
%token <str> NUMBER_VAL STRING_VAL ID
%token LET PRINT INPUT IF ELSE WHILE FOR
%token LET_ONCE HISTORY BENCHMARK DRAW_CHART DUMP_MEMORY
%token REACTIVE MONEY RETRY ON_FAIL CLEAN_CODE SAFE_EXTRACT ASYNC_TASK VALIDATE
%token PLUS MINUS MUL DIV ASSIGN EQ NEQ GT LT GTE LTE PIPE
%token SEMI COMMA LPAREN RPAREN LBRACE RBRACE LBRACKET RBRACKET DOT

%type <ast> expr statement statement_list program block simple_assign
%type <ast> control_flow advanced_blocks

/* Operator Precedence */
%left EQ NEQ GT LT GTE LTE
%left PLUS MINUS
%left MUL DIV

%%
program:
    statement_list { root = $1; }
    ;

statement_list:
    statement { $$ = $1; }
    | statement_list statement { $$ = new_op_node(NODETYPE_BLOCK, $1, $2); }
    ;

block:
    LBRACE statement_list RBRACE { $$ = $2; }
    ;

simple_assign:
    ID ASSIGN expr { $$ = new_assign_node($1, $3, ASSIGN_NORMAL); }
    ;

statement:
    expr SEMI { $$ = $1; }
    | PRINT expr SEMI { $$ = new_print_node($2); }
    | INPUT ID SEMI { $$ = new_input_node($2); }
    | LET ID ASSIGN expr SEMI { $$ = new_assign_node($2, $4, ASSIGN_NORMAL); }
    | ID ASSIGN expr SEMI { $$ = new_assign_node($1, $3, ASSIGN_NORMAL); }
    | control_flow { $$ = $1; }
    | advanced_blocks { $$ = $1; }
    ;

control_flow:
    
    IF LPAREN expr RPAREN block { $$ = new_if_node($3, $5, NULL); }
    | IF LPAREN expr RPAREN block ELSE block { $$ = new_if_node($3, $5, $7); }
    | WHILE LPAREN expr RPAREN block { $$ = new_while_node($3, $5); }
    | FOR LPAREN statement expr SEMI simple_assign RPAREN block {
          ASTNode* body_with_update = new_op_node(NODETYPE_BLOCK, $8, $6);
          ASTNode* while_loop = new_while_node($4, body_with_update);
          $$ = new_op_node(NODETYPE_BLOCK, $3, while_loop);
      }
    ;

advanced_blocks:
    
    REACTIVE ID ASSIGN expr SEMI { $$ = new_assign_node($2, $4, ASSIGN_REACTIVE); }
    | LET_ONCE ID ASSIGN expr SEMI { $$ = new_assign_node($2, $4, ASSIGN_QUANTUM); }
    | MONEY ID ASSIGN expr SEMI { $$ = new_assign_node($2, $4, ASSIGN_MONEY); }
    | HISTORY LPAREN ID RPAREN SEMI { $$ = new_special_node(NODETYPE_HISTORY, new_var_node($3), NULL); }
    | BENCHMARK block { $$ = new_special_node(NODETYPE_BENCHMARK, $2, NULL); }
    | RETRY LPAREN NUMBER_VAL RPAREN block { $$ = new_retry_node($5, NULL, atoi($3)); }
    | RETRY LPAREN NUMBER_VAL RPAREN block ON_FAIL block { $$ = new_retry_node($5, $7, atoi($3)); }
    | SAFE_EXTRACT LPAREN ID RPAREN SEMI { $$ = new_special_node(NODETYPE_SAFE_EXTRACT, new_var_node($3), NULL); }
    | DRAW_CHART LPAREN ID RPAREN SEMI { $$ = new_special_node(NODETYPE_DRAW_CHART, new_var_node($3), NULL); }
    | DUMP_MEMORY LPAREN RPAREN SEMI { $$ = new_special_node(NODETYPE_DUMP_MEMORY, NULL, NULL); }
    ;

expr:
    NUMBER_VAL        { $$ = new_num_node(atof($1)); }
    | STRING_VAL      { $$ = new_string_node($1); }
    | ID              { $$ = new_var_node($1); }
    | expr PLUS expr  { $$ = new_op_node(NODETYPE_ADD, $1, $3); }
    | expr MINUS expr { $$ = new_op_node(NODETYPE_SUB, $1, $3); }
    | expr MUL expr   { $$ = new_op_node(NODETYPE_MUL, $1, $3); }
    | expr DIV expr   { $$ = new_op_node(NODETYPE_DIV, $1, $3); }
    | expr GT expr    { $$ = new_op_node(NODETYPE_GT, $1, $3); }
    | expr LT expr    { $$ = new_op_node(NODETYPE_LT, $1, $3); }
    | expr GTE expr   { $$ = new_op_node(NODETYPE_GTE, $1, $3); }
    | expr LTE expr   { $$ = new_op_node(NODETYPE_LTE, $1, $3); }
    | expr EQ expr    { $$ = new_op_node(NODETYPE_EQ, $1, $3); }
    | expr NEQ expr   { $$ = new_op_node(NODETYPE_NEQ, $1, $3); }
    | LPAREN expr RPAREN { $$ = $2; }
    ;
%%

void yyerror(const char *s) {
    extern char *yytext;
    printf("[Syntax Error] %s, at token: '%s'\n", s, yytext);
}

int main(int argc, char** argv) {
    char* filename = NULL;
    for(int i = 1; i < argc; i++) {
        if(strcmp(argv[i], "--explain") == 0) {
            explain_mode = 1;
        } else {
            filename = argv[i];
        }
    }
    if(!filename) {
        printf("[Error] Usage: hydrop.exe <script.jl> [--explain]\n");
        return 1;
    }
    yyin = fopen(filename, "r");
    if(!yyin) {
        printf("[Error] Could not open script file: %s\n", filename);
        return 1;
    }

    printf("==========================================\n");
    printf("   HyDrop.jl Compiler (Parsing Engine)\n");
    printf("==========================================\n");
    yyparse();

    execute_ast(root);

    fclose(yyin);
    return 0;
}
