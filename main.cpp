#include "output.hpp"
#include "nodes.hpp"
#include "semantic.hpp"
#include "ir.hpp"
#include <iostream>

// Extern from the bison-generated parser
extern int yyparse();

extern std::shared_ptr<ast::Node> program;

int main() {
    // Parse the input. The result is stored in the global variable `program`
    yyparse();

	semantic::SemanticVisitor semanticVisitor;
	program->accept(semanticVisitor);
	//std::cout<<semanticVisitor.printer;

	Ir::IrVisitor irVisitor;
	program->accept(irVisitor);	
	std::cout<<irVisitor.codebuffer;

    return 0;
}
