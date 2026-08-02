%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.tab.h"
%}

%option noyywrap

%%
    /* ========================================================
       PHASE 1: Core Fundamentals & Professor's Requirements
       ======================================================== */
"let"           { return LET; }
"show"          { return PRINT; }
"input"         { return INPUT; }
"if"            { return IF; }
"else"          { return ELSE; }
"while"         { return WHILE; }
"for"           { return FOR; }

    /* ========================================================
       PHASE 3: Security & Stability
       ======================================================== */
"let_once"      { return LET_ONCE; }    /* Quantum Variable */

    /* ========================================================
       PHASE 4: Advanced Software Engineering
       ======================================================== */
"history"       { return HISTORY; }     /* Time Travel Variables */
"benchmark"     { return BENCHMARK; }   /* Execution timer block */
"draw_chart"    { return DRAW_CHART; }  /* ASCII Bar charts */
"dump_memory"   { return DUMP_MEMORY; } 

    /* ========================================================
       PHASE 5: Modern Industry Solutions
       ======================================================== */
"reactive"      { return REACTIVE; }    /* Auto-updating variable */
"money"         { return MONEY; }       /* Fixed 2-decimal precision type */
"retry"         { return RETRY; }       /* Auto-retry block */
"on_fail"       { return ON_FAIL; }     /* Fallback block for retry */

    /* ========================================================
       PHASE 6 & 7: Architecture & Advanced Real-World
       ======================================================== */
"clean_code"    { return CLEAN_CODE; }  /* Dead code elimination */
"safe_extract"  { return SAFE_EXTRACT; }/* Safe JSON destructuring */
"async_task"    { return ASYNC_TASK; }  /* Thread-safe block */
"validate"      { return VALIDATE; }    /* Data validation pipe */


    /* ========================================================
       Operators & Logic (Professor's Requirements)
       ======================================================== */
"+"             { return PLUS; }
"-"             { return MINUS; }
"*"             { return MUL; }
"/"             { return DIV; }
"="             { return ASSIGN; }
"=="            { return EQ; }
"!="            { return NEQ; }
">"             { return GT; }
"<"             { return LT; }
">="            { return GTE; }
"<="            { return LTE; }
"|>"            { return PIPE; }        /* Pipeline operator */


    /* ========================================================
       Identifiers, Numbers, Strings & Data Types
       ======================================================== */
    /* Number with optional decimal and optional unit (km, m) */
[0-9]+(\.[0-9]+)?(km|m)? {
                    float value = atof(yytext);
                    int len = strlen(yytext);
                    if(len >= 2 && yytext[len-2]=='k' && yytext[len-1]=='m') {
                        value = value * 1000.0;   /* km -> base unit (m) */
                    }
                    /* plain "m" suffix: already base unit, value unchanged */
                    char buf[64];
                    snprintf(buf, sizeof(buf), "%g", value);
                    yylval.str = strdup(buf);
                    return NUMBER_VAL;
                }

    /* Strings inside quotes (quotes stripped, \n and \t escapes processed) */
\"[^\"]*\"      {
                    char *raw = yytext + 1;               /* skip leading " */
                    int len = strlen(raw) - 1;             /* drop trailing " */
                    char *buf = malloc(len + 1);
                    int j = 0;
                    for(int i = 0; i < len; i++) {
                        if(raw[i] == '\\' && i+1 < len && raw[i+1] == 'n') {
                            buf[j++] = '\n';
                            i++;
                        } else if(raw[i] == '\\' && i+1 < len && raw[i+1] == 't') {
                            buf[j++] = '\t';
                            i++;
                        } else {
                            buf[j++] = raw[i];
                        }
                    }
                    buf[j] = '\0';
                    yylval.str = buf;
                    return STRING_VAL;
                }

    /* Variable Names / Identifiers */
[a-zA-Z_][a-zA-Z0-9_]* {
                    yylval.str = strdup(yytext);
                    return ID;
                }


    /* ========================================================
       Syntax Symbols & Ignore Rules
       ======================================================== */
";"             { return SEMI; }
","             { return COMMA; }
"("             { return LPAREN; }
")"             { return RPAREN; }
"{"             { return LBRACE; }
"}"             { return RBRACE; }
"["             { return LBRACKET; }
"]"             { return RBRACKET; }
"."             { return DOT; }

"//".*                       { /* Ignore single-line comments */ }
"/*"([^*]|\*+[^*/])*\*+"/"   { /* Ignore multi-line comments */ }
[ \t\n\r]+      { /* Ignore spaces, tabs, and newlines */ }

.               { printf("[Lexer Error] Unknown symbol found: %s\n", yytext); }
%%
