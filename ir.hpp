#ifndef IR_HPP
#define IR_HPP

#include <vector>
#include <string>
#include <stack>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <climits>
#include "visitor.hpp"
#include "nodes.hpp"
#include "output.hpp"
#include <fstream>


namespace Ir{

    /* IrVisitor class
     * This class is used to generate llvm IR code
     */

    class IrVisitor : public Visitor {
    private:
	
    public:
		
    	output::CodeBuffer codebuffer;

		IrVisitor();

		void allocate_arguments_on_stack(ast::Formals &node);

        void visit(ast::Num &node) override;

        void visit(ast::NumB &node) override;

        void visit(ast::String &node) override;

        void visit(ast::Bool &node) override;

        void visit(ast::ID &node) override;

        void visit(ast::BinOp &node) override;

        void visit(ast::RelOp &node) override;

        void visit(ast::Not &node) override;

        void visit(ast::And &node) override;

        void visit(ast::Or &node) override;

        void visit(ast::Type &node) override;

        void visit(ast::Cast &node) override;

        void visit(ast::ExpList &node) override;

        void visit(ast::Call &node) override;

        void visit(ast::Statements &node) override;

        void visit(ast::Break &node) override;

        void visit(ast::Continue &node) override;

        void visit(ast::Return &node) override;

        void visit(ast::If &node) override;

        void visit(ast::While &node) override;

        void visit(ast::VarDecl &node) override;

        void visit(ast::Assign &node) override;

        void visit(ast::Formal &node) override;

        void visit(ast::Formals &node) override;

        void visit(ast::FuncDecl &node) override;

        void visit(ast::Funcs &node) override;
    };
}

#endif // IR_HPP
