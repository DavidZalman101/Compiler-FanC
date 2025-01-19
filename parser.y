%{

#include "nodes.hpp"
#include "output.hpp"
#include <iostream>

// bison declarations
extern int yylineno;
extern int yylex();
ast::BinOpType currentBinOp;
ast::RelOpType currentRelOp;

void yyerror(const char*);

// root of the AST, set by the parser and used by other parts of the compiler
std::shared_ptr<ast::Node> program;

using namespace std;

// TODO: Place any additional declarations here
%}

// TODO: Define tokens here
%token BYTE BOOL INT VOID TRUE FALSE RETURN IF WHILE BREAK CONTINUE SC COMMA ID NUM NUM_B STRING

// TODO: Define precedence and associativity here
%right ASSIGN
%left OR
%left AND
%left RELOP
%left BINOP_ADD BINOP_SUB
%left BINOP_MUL BINOP_DIV
%right NOT
%left LPAREN RPAREN LBRACE RBRACE
// Solve the Dangling else conflict
%nonassoc ELSE

%%

// While reducing the start variable, set the root of the AST
Program:  Funcs { program = $1; }
;

// TODO: Define grammar here

Funcs	:	/* epsilon */ { $$ = std::make_shared<ast::Funcs>(); }
      	|	FuncDecl Funcs {
			$$ = $2;
			std::dynamic_pointer_cast<ast::Funcs>($$)->push_front(std::dynamic_pointer_cast<ast::FuncDecl>($1)); }	
	;

FuncDecl:	RetType ID LPAREN Formals RPAREN LBRACE Statements RBRACE {
			$$ = std::make_shared<ast::FuncDecl>(std::dynamic_pointer_cast<ast::ID>($2),
							     std::dynamic_pointer_cast<ast::Type>($1),
							     std::dynamic_pointer_cast<ast::Formals>($4),
							     std::dynamic_pointer_cast<ast::Statements>($7)); }
	;

RetType:	Type { $$ = $1; }
       	|	VOID { $$ = std::make_shared<ast::Type>(ast::BuiltInType::VOID); }
	;

Formals:	/* epsilon */ { $$ = std::make_shared<ast::Formals>(); }
       	|	FormalsList { $$ = $1; }
	;

FormalsList:	FormalDecl {
	   		$$ = std::make_shared<ast::Formals>(std::dynamic_pointer_cast<ast::Formal>($1)); }
	|	FormalDecl COMMA FormalsList {
			$$ = $3;
			std::dynamic_pointer_cast<ast::Formals>($$)->push_front(std::dynamic_pointer_cast<ast::Formal>($1)); }
	;

FormalDecl:	Type ID {
	  		$$ = std::make_shared<ast::Formal>(std::dynamic_pointer_cast<ast::ID>($2),
							   std::dynamic_pointer_cast<ast::Type>($1)); }
	  ;

Statements:	Statement {
	  		$$ = std::make_shared<ast::Statements>(std::dynamic_pointer_cast<ast::Statement>($1)); }
	  |	Statements Statement {
			$$ = $1;
			std::dynamic_pointer_cast<ast::Statements>($$)->push_back(std::dynamic_pointer_cast<ast::Statement>($2)); }
	  ;

Statement:	LBRACE Statements RBRACE {
			std::dynamic_pointer_cast<ast::Statement>($2)->block = true;
			$$ = $2; }
	|	Type ID SC {
			$$ = std::make_shared<ast::VarDecl>(std::dynamic_pointer_cast<ast::ID>($2),
							    std::dynamic_pointer_cast<ast::Type>($1)); }
	|	Type ID ASSIGN Exp SC {
			$$ = std::make_shared<ast::VarDecl>(std::dynamic_pointer_cast<ast::ID>($2),
							    std::dynamic_pointer_cast<ast::Type>($1),
							    std::dynamic_pointer_cast<ast::Exp>($4)); }
	|	ID ASSIGN Exp SC {
			$$ = std::make_shared<ast::Assign>(std::dynamic_pointer_cast<ast::ID>($1),
							   std::dynamic_pointer_cast<ast::Exp>($3)); }
	|	Call SC { $$ = $1; }
	|	RETURN SC { 
			$$ = std::make_shared<ast::Return>(); }
	|	RETURN Exp SC {
			$$ = std::make_shared<ast::Return>(std::dynamic_pointer_cast<ast::Exp>($2)); }
	| 	IF LPAREN Exp RPAREN Statement {
			$$ = std::make_shared<ast::If>(std::dynamic_pointer_cast<ast::Exp>($3),
						       std::dynamic_pointer_cast<ast::Statement>($5));
			}
	|	IF LPAREN Exp RPAREN Statement ELSE Statement {
			$$ = std::make_shared<ast::If>(std::dynamic_pointer_cast<ast::Exp>($3),
						       std::dynamic_pointer_cast<ast::Statement>($5),
					 	       std::dynamic_pointer_cast<ast::Statement>($7)); }
	|	WHILE LPAREN Exp RPAREN Statement {
			$$ = std::make_shared<ast::While>(std::dynamic_pointer_cast<ast::Exp>($3),
							  std::dynamic_pointer_cast<ast::Statement>($5)); }
	|	BREAK SC {
			$$ = std::make_shared<ast::Break>(); }
	|	CONTINUE SC { $$ = std::make_shared<ast::Continue>(); }
	;

Call:		ID LPAREN ExpList RPAREN {
    			$$ = std::make_shared<ast::Call>(std::dynamic_pointer_cast<ast::ID>($1),
							 std::dynamic_pointer_cast<ast::ExpList>($3)); }
    	|	ID LPAREN RPAREN {
			$$ = std::make_shared<ast::Call>(std::dynamic_pointer_cast<ast::ID>($1)); }
	;

ExpList:	Exp { $$ = std::make_shared<ast::ExpList>(std::dynamic_pointer_cast<ast::Exp>($1)); }
       	|	Exp COMMA ExpList { $$ = $3;
			std::dynamic_pointer_cast<ast::ExpList>($$)->push_front(std::dynamic_pointer_cast<ast::Exp>($1)); }
	;

Type:		INT { $$ = $1; }
    	|	BYTE { $$ = $1; }
	|	BOOL { $$ = $1; }
	;

Exp:		LPAREN Exp RPAREN { $$ = $2; }
   	|	Exp BINOP_MUL Exp {
			$$ = std::make_shared<ast::BinOp>(std::dynamic_pointer_cast<ast::Exp>($1),
								  std::dynamic_pointer_cast<ast::Exp>($3),
								  ast::BinOpType::MUL); }
	|	Exp BINOP_DIV Exp {
			$$ = std::make_shared<ast::BinOp>(std::dynamic_pointer_cast<ast::Exp>($1),
								  std::dynamic_pointer_cast<ast::Exp>($3),
								  ast::BinOpType::DIV); }
	|	Exp BINOP_ADD Exp {
			$$ = std::make_shared<ast::BinOp>(std::dynamic_pointer_cast<ast::Exp>($1),
								  std::dynamic_pointer_cast<ast::Exp>($3),
								  ast::BinOpType::ADD); } 
	|	Exp BINOP_SUB Exp {
			$$ = std::make_shared<ast::BinOp>(std::dynamic_pointer_cast<ast::Exp>($1),
								  std::dynamic_pointer_cast<ast::Exp>($3),
								  ast::BinOpType::SUB); }
	|	ID { $$ = $1; }
	|	Call { $$ = $1; }
	|	NUM { $$ = $1; }
	|	NUM_B { $$ = $1; }
	|	STRING { $$ = $1; }
	|	TRUE { $$ = $1; }
	|	FALSE { $$ = $1; }
	|	NOT Exp { $$ = std::make_shared<ast::Not>(std::dynamic_pointer_cast<ast::Exp>($2)); }
	|	Exp AND Exp {
			$$ = std::make_shared<ast::And>(std::dynamic_pointer_cast<ast::Exp>($1),
							std::dynamic_pointer_cast<ast::Exp>($3)); }
	|	Exp OR Exp {
			$$ = std::make_shared<ast::Or>(std::dynamic_pointer_cast<ast::Exp>($1),
						       std::dynamic_pointer_cast<ast::Exp>($3)); }
	|	Exp RELOP Exp {
			if (currentRelOp == ast::EQ)
				$$ = std::make_shared<ast::RelOp>(std::dynamic_pointer_cast<ast::Exp>($1),
								  std::dynamic_pointer_cast<ast::Exp>($3),
								  ast::EQ);
			else if (currentRelOp == ast::NE)
				$$ = std::make_shared<ast::RelOp>(std::dynamic_pointer_cast<ast::Exp>($1),
								  std::dynamic_pointer_cast<ast::Exp>($3),
								  ast::NE);
			else if (currentRelOp == ast::LT)
				$$ = std::make_shared<ast::RelOp>(std::dynamic_pointer_cast<ast::Exp>($1),
								  std::dynamic_pointer_cast<ast::Exp>($3),
								  ast::LT);
			else if (currentRelOp == ast::GT)
				$$ = std::make_shared<ast::RelOp>(std::dynamic_pointer_cast<ast::Exp>($1),
								  std::dynamic_pointer_cast<ast::Exp>($3),
								  ast::GT);
			else if (currentRelOp == ast::LE)
				$$ = std::make_shared<ast::RelOp>(std::dynamic_pointer_cast<ast::Exp>($1),
								  std::dynamic_pointer_cast<ast::Exp>($3),
								  ast::LE);
			else
				$$ = std::make_shared<ast::RelOp>(std::dynamic_pointer_cast<ast::Exp>($1),
								  std::dynamic_pointer_cast<ast::Exp>($3),
								  ast::GE); }
	|	LPAREN Type RPAREN Exp {
			$$ = std::make_shared<ast::Cast>(std::dynamic_pointer_cast<ast::Exp>($4),
							 std::dynamic_pointer_cast<ast::Type>($2)); }
	;
%%

// TODO: Place any additional code here
void yyerror(const char *message)
{
	std::string s(message);
	output::errorSyn(yylineno);
	exit(0);
}
