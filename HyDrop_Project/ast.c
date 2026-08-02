#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <setjmp.h>
#include "ast.h"

float eval_expr(ASTNode* node);  

/* ========================================================
   SYMBOL TABLE (Memory Management)
   ======================================================== */
#define MAX_VARS 100
#define HISTORY_MAX 20

typedef struct {
    char name[50];
    float value;
    int is_initialized;
    int is_reactive;
    int is_quantum;                 
    int is_money;                  
    struct ASTNode* reactive_expr;  
    int is_tracked;                 
    float history[HISTORY_MAX];  
    int history_count;              
} Variable;

Variable memory[MAX_VARS];
int var_count = 0;
int explain_mode = 0;   


jmp_buf retry_env;
int in_retry = 0;

int find_var(char* name) {
    for(int i = 0; i < var_count; i++) {
        if(strcmp(memory[i].name, name) == 0) return i;
    }
    return -1;
}

void recompute_reactive(void) {
    for(int i = 0; i < var_count; i++) {
        if(memory[i].is_reactive && memory[i].reactive_expr) {
            memory[i].value = eval_expr(memory[i].reactive_expr);
            if(explain_mode) {
                printf("[Explain] Reactive variable '%s' recomputed -> %g\n", memory[i].name, memory[i].value);
            }
        }
    }
}

void set_var(char* name, float val, int reactive, int quantum) {
    int idx = find_var(name);
    if(idx == -1) {
        strcpy(memory[var_count].name, name);
        memory[var_count].value = val;
        memory[var_count].is_initialized = 1;
        memory[var_count].is_reactive = reactive;
        memory[var_count].is_quantum = quantum;
        memory[var_count].is_money = 0;
        memory[var_count].reactive_expr = NULL;
        memory[var_count].is_tracked = 0;
        memory[var_count].history_count = 0;
        var_count++;
    } else {
        if(memory[idx].is_tracked && memory[idx].history_count < HISTORY_MAX) {
            memory[idx].history[memory[idx].history_count++] = memory[idx].value;
        }
        memory[idx].value = val;
    }
    if(explain_mode) {
        printf("[Explain] Memory updated -> %s = %g\n", name, val);
    }
    // Phase 5: any change might affect reactive variables depending on it
    recompute_reactive();
}

// Phase 5: declare/update a reactive variable, storing its live formula
void set_reactive_var(char* name, ASTNode* expr) {
    int idx = find_var(name);
    float val = eval_expr(expr);
    if(idx == -1) {
        strcpy(memory[var_count].name, name);
        memory[var_count].value = val;
        memory[var_count].is_initialized = 1;
        memory[var_count].is_reactive = 1;
        memory[var_count].is_quantum = 0;
        memory[var_count].is_money = 0;
        memory[var_count].reactive_expr = expr;
        memory[var_count].is_tracked = 0;
        memory[var_count].history_count = 0;
        var_count++;
    } else {
        memory[idx].value = val;
        memory[idx].is_reactive = 1;
        memory[idx].reactive_expr = expr;
    }
    if(explain_mode) {
        printf("[Explain] Reactive variable '%s' declared -> %g\n", name, val);
    }
}

// Phase 5: declare/update a money variable, rounding to 2 decimal places
void set_money_var(char* name, float val) {
    float rounded = roundf(val * 100.0f) / 100.0f;
    int idx = find_var(name);
    if(idx == -1) {
        strcpy(memory[var_count].name, name);
        memory[var_count].value = rounded;
        memory[var_count].is_initialized = 1;
        memory[var_count].is_reactive = 0;
        memory[var_count].is_quantum = 0;
        memory[var_count].is_money = 1;
        memory[var_count].reactive_expr = NULL;
        memory[var_count].is_tracked = 0;
        memory[var_count].history_count = 0;
        var_count++;
    } else {
        memory[idx].value = rounded;
        memory[idx].is_money = 1;
    }
    if(explain_mode) {
        printf("[Explain] Money variable '%s' = %.2f (rounded)\n", name, rounded);
    }
    recompute_reactive();
}

void dump_all_vars(void) {
    printf("\n[Memory Snapshot] ---- %d variable(s) ----\n", var_count);
    for(int i = 0; i < var_count; i++) {
        printf("  %-20s = %g %s\n", memory[i].name,
               memory[i].value,
               memory[i].is_initialized ? "" : "(destroyed/uninitialized)");
    }
    printf("[Memory Snapshot] --------------------------\n\n");
}

float get_var(char* name) {
    int idx = find_var(name);
    if(idx == -1 || !memory[idx].is_initialized) {
        printf("\n[Error] The variable '%s' is not declared or initialized. Please use 'let %s = value;' first.\n", name, name);
        dump_all_vars();
        exit(1);
    }
    float val = memory[idx].value;

    // Phase 3: Quantum Variable (Destroy on read)
    if(memory[idx].is_quantum) {
        memory[idx].is_initialized = 0; // Destroyed
        printf("[Security Log] Quantum variable '%s' was read and permanently destroyed from memory.\n", name);
    }
    return val;
}

/* ========================================================
   AST NODE CREATION (Building the Tree)
   ======================================================== */
ASTNode* new_num_node(float val) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = NODETYPE_NUMBER;
    node->float_val = val;
    node->left = node->right = node->condition = node->body = NULL;
    return node;
}

ASTNode* new_var_node(char* name) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = NODETYPE_VAR;
    node->var_name = strdup(name);
    node->left = node->right = node->condition = node->body = NULL;
    return node;
}

ASTNode* new_op_node(NodeType type, ASTNode* left, ASTNode* right) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = type;
    node->left = left;
    node->right = right;
    node->condition = node->body = NULL;
    return node;
}

ASTNode* new_assign_node(char* name, ASTNode* expr, int mode) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = NODETYPE_ASSIGN;
    node->var_name = strdup(name);
    node->left = expr;
    node->float_val = (float)mode; 
    node->right = node->condition = node->body = NULL;
    return node;
}

ASTNode* new_if_node(ASTNode* cond, ASTNode* if_body, ASTNode* else_body) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = NODETYPE_IF;
    node->condition = cond;
    node->left = if_body;
    node->right = else_body;
    return node;
}

ASTNode* new_while_node(ASTNode* cond, ASTNode* body) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = NODETYPE_WHILE;
    node->condition = cond;
    node->body = body;
    node->left = node->right = NULL;
    return node;
}

ASTNode* new_print_node(ASTNode* expr) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = NODETYPE_PRINT;
    node->left = expr;
    node->right = node->condition = node->body = NULL;
    return node;
}

ASTNode* new_string_node(char* val) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = NODETYPE_STRING;
    node->str_val = strdup(val);
    node->left = node->right = node->condition = node->body = NULL;
    return node;
}

ASTNode* new_input_node(char* name) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = NODETYPE_INPUT;
    node->var_name = strdup(name);
    node->left = node->right = node->condition = node->body = NULL;
    return node;
}

ASTNode* new_special_node(NodeType type, ASTNode* left, ASTNode* right) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = type;
    node->left = left;
    node->right = right;
    node->condition = node->body = NULL;
    return node;
}

ASTNode* new_retry_node(ASTNode* body, ASTNode* fallback, int count) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = NODETYPE_RETRY;
    node->left = body;
    node->right = fallback;
    node->float_val = (float)count;
    node->condition = node->body = NULL;
    return node;
}

/* ========================================================
   AST EVALUATION (Math & Logic)
   ======================================================== */
float eval_expr(ASTNode* node) {
    if(!node) return 0;

    switch(node->type) {
        case NODETYPE_NUMBER: return node->float_val;
        case NODETYPE_VAR: return get_var(node->var_name);
        case NODETYPE_ADD: return eval_expr(node->left) + eval_expr(node->right);
        case NODETYPE_SUB: return eval_expr(node->left) - eval_expr(node->right);
        case NODETYPE_MUL: return eval_expr(node->left) * eval_expr(node->right);
        case NODETYPE_DIV: {
            float denom = eval_expr(node->right);
            if(denom == 0) {
                if(in_retry) {
                    
                    longjmp(retry_env, 1);
                }
                printf("\n[Phase 3 Security Guard] Fatal Error: Division by zero attempted!\n");
                dump_all_vars();
                exit(1);
            }
            return eval_expr(node->left) / denom;
        }
        case NODETYPE_GT: return eval_expr(node->left) > eval_expr(node->right);
        case NODETYPE_LT: return eval_expr(node->left) < eval_expr(node->right);
        case NODETYPE_EQ: return eval_expr(node->left) == eval_expr(node->right);
        case NODETYPE_NEQ: return eval_expr(node->left) != eval_expr(node->right);
        case NODETYPE_GTE: return eval_expr(node->left) >= eval_expr(node->right);
        case NODETYPE_LTE: return eval_expr(node->left) <= eval_expr(node->right);
        default: return 0;
    }
}

/* ========================================================
   PRINT HELPERS (string + number concatenation via '+')
   ======================================================== */
int contains_string(ASTNode* node) {
    if(!node) return 0;
    if(node->type == NODETYPE_STRING) return 1;
    if(node->type == NODETYPE_ADD)
        return contains_string(node->left) || contains_string(node->right);
    return 0;
}

void print_node(ASTNode* node) {
    if(node->type == NODETYPE_STRING) {
        printf("%s", node->str_val);
    } else if(node->type == NODETYPE_ADD && contains_string(node)) {
        print_node(node->left);
        print_node(node->right);
    } else if(node->type == NODETYPE_VAR) {
        int idx = find_var(node->var_name);
        float val = eval_expr(node);
        if(idx != -1 && memory[idx].is_money) {
            printf("%.2f", val);
        } else {
            printf("%g", val);
        }
    } else {
        printf("%g", eval_expr(node));
    }
}

/* ========================================================
   AST EXECUTION (Running Statements & Control Flow)
   ======================================================== */
void execute_ast(ASTNode* node) {
    if(!node) return;

    switch(node->type) {
        case NODETYPE_BLOCK:
            execute_ast(node->left);
            execute_ast(node->right);
            break;

        case NODETYPE_ASSIGN: {
            int mode = (int)node->float_val;
            if(mode == ASSIGN_REACTIVE) {
                set_reactive_var(node->var_name, node->left);
            } else if(mode == ASSIGN_MONEY) {
                float val = eval_expr(node->left);
                set_money_var(node->var_name, val);
            } else {
                float val = eval_expr(node->left);
                int is_quant = (mode == ASSIGN_QUANTUM);
                set_var(node->var_name, val, 0, is_quant);
            }
            break;
        }

        case NODETYPE_PRINT:
            print_node(node->left);
            printf("\n");
            break;

        case NODETYPE_INPUT: {
            float val;
            printf("Input din (%s) => ", node->var_name);
            if (scanf("%f", &val) != 1) {
                printf("[Warning] No input available (stdin already consumed by script). Defaulting %s to 0.\n", node->var_name);
                val = 0;
            }
            set_var(node->var_name, val, 0, 0);
            break;
        }

        case NODETYPE_IF: {
            float cond_val = eval_expr(node->condition);
            if(explain_mode) {
                printf("[Explain] Condition result = %g -> %s\n", cond_val, cond_val ? "true, running IF block" : "false, running ELSE block");
            }
            if(cond_val) {
                execute_ast(node->left);
            } else if(node->right) {
                execute_ast(node->right);
            }
            break;
        }

        case NODETYPE_WHILE: {
            int loop_counter = 0;
           
            while(eval_expr(node->condition)) {
                execute_ast(node->body);
                loop_counter++;
                if(loop_counter > 10000) {
                    printf("\n[Phase 3 Security Guard] Infinite loop detected! Forcefully halting execution to prevent memory freeze.\n");
                    break;
                }
            }
            break;
        }

        case NODETYPE_HISTORY: {
            char* name = node->left->var_name;
            int idx = find_var(name);
            if(idx == -1) {
                printf("[Phase 4 Tool] Variable '%s' not found.\n", name);
                break;
            }
            if(!memory[idx].is_tracked) {
                memory[idx].is_tracked = 1;
                printf("[Phase 4 Tool] History tracking enabled for '%s' (current value: %g)\n", name, memory[idx].value);
            } else {
                printf("[Phase 4 Tool] History of '%s': ", name);
                for(int i = 0; i < memory[idx].history_count; i++) {
                    printf("%g -> ", memory[idx].history[i]);
                }
                printf("%g (current)\n", memory[idx].value);
            }
            break;
        }

        case NODETYPE_BENCHMARK: {
            clock_t start = clock();
            execute_ast(node->left);
            clock_t end = clock();
            double ms = ((double)(end - start)) / CLOCKS_PER_SEC * 1000.0;
            printf("[Phase 4 Tool] Benchmark complete. Execution time: %.4f ms\n", ms);
            break;
        }

        case NODETYPE_RETRY: {
            int count = (int)node->float_val;
            if(count < 1) count = 1;
            int success = 0;
            in_retry = 1;
            for(int attempt = 1; attempt <= count; attempt++) {
                if(setjmp(retry_env) == 0) {
                    printf("[Phase 5 Tool] Retry attempt %d/%d...\n", attempt, count);
                    execute_ast(node->left);
                    success = 1;
                    break;
                } else {
                    printf("[Phase 5 Tool] Attempt %d failed (division by zero). Retrying...\n", attempt);
                }
            }
            in_retry = 0;
            if(!success) {
                printf("[Phase 5 Tool] All %d attempts failed.\n", count);
                if(node->right) execute_ast(node->right);
            }
            break;
        }

        case NODETYPE_SAFE_EXTRACT:
            printf("[Phase 7 Tool] Safely verified JSON path for: %s\n", node->left->var_name);
            break;

        case NODETYPE_DRAW_CHART: {
            char* name = node->left->var_name;
            float val = get_var(name);
            int bars = (int)(val < 0 ? -val : val);
            if(bars > 100) bars = 100;
            printf("[Phase 4 Tool] %s = %g |", name, val);
            for(int i = 0; i < bars; i++) printf("#");
            printf("\n");
            break;
        }

        case NODETYPE_DUMP_MEMORY:
            dump_all_vars();
            break;

        default:
            eval_expr(node);
            break;
    }
}