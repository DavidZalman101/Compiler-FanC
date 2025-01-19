%{
/* Declatations section */
#include "output.hpp"
#include "parser.tab.h" // needs access to yylval from bison
#include "nodes.hpp"
#include <iostream>

extern ast::BinOpType currentBinOp;
extern ast::RelOpType currentRelOp;
%}

%option yylineno
%option noyywrap

%%
[\t\n\r ] ;
void { return VOID; }
and {  return AND; }
or { return OR; }
not { return NOT; }
true { yylval = std::make_shared<ast::Bool>(true); return TRUE; }
false { yylval = std::make_shared<ast::Bool>(false); return FALSE; }
return { return RETURN; }
if { return IF; }
else { return ELSE; }
while { return WHILE; }
break { return BREAK; }
continue { return CONTINUE; }
int { yylval = std::make_shared<ast::Type>(ast::BuiltInType::INT); return INT; }
byte { yylval = std::make_shared<ast::Type>(ast::BuiltInType::BYTE); return BYTE; }
bool { yylval = std::make_shared<ast::Type>(ast::BuiltInType::BOOL); return BOOL; }
; { return SC; }
, { return COMMA; }
\( { return LPAREN; }
\) { return RPAREN; }
\{ { return LBRACE; }
\} { return RBRACE; }
= { return ASSIGN; }
"==" { currentRelOp = ast::EQ; return RELOP; }
"!=" { currentRelOp = ast::NE; return RELOP; }
"<" { currentRelOp = ast::LT; return RELOP; }
">" { currentRelOp = ast::GT; return RELOP; }
"<=" { currentRelOp = ast::LE; return RELOP; }
">=" { currentRelOp = ast::GE; return RELOP; }
\+ { currentBinOp = ast::ADD; return BINOP_ADD; }
\- { currentBinOp = ast::SUB; return BINOP_SUB; }
\* { currentBinOp = ast::MUL; return BINOP_MUL; }
\/ { currentBinOp = ast::DIV; return BINOP_DIV; }
[a-zA-Z][a-zA-Z0-9]* { yylval = std::make_shared<ast::ID>(yytext); return ID; }
(0|[1-9][0-9]*) { yylval = std::make_shared<ast::Num>(yytext); return NUM; }
(0b|[1-9][0-9]*b) { yylval = std::make_shared<ast::NumB>(yytext); return NUM_B; }
\"([^\n\r\"\\]|\\[rnt\"\\])+\" { yylval = std::make_shared<ast::String>(yytext); return STRING; }
\/\/[^\r\n]* {}
. { output::errorLex(yylineno); }
