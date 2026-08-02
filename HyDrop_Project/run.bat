@echo off
del parser.tab.c parser.tab.h lex.yy.c hydrop.exe 2>nul

echo [1/3] Bison theke Parser toiri hocche...
C:\sdk\win_bison -d parser.y

echo [2/3] Flex theke Lexer toiri hocche...
C:\sdk\win_flex lexer.l

echo [3/3] GCC diye C code Compile hocche...
gcc parser.tab.c lex.yy.c ast.c -o hydrop.exe

echo.
echo ===========================================
echo   COMPILATION SUCCESS! HYDROP IS RUNNING   
echo ===========================================
echo.

if "%~1"=="" (
    set /p SCRIPT_FILE="which file you want to run? (e.g. test1.jl): "
) else (
    set SCRIPT_FILE=%~1
)

.\hydrop.exe %SCRIPT_FILE%