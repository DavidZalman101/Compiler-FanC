#ifndef SEMANTIC_HPP
#define SEMANTIC_HPP

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


namespace semantic {

	struct func {
		std::string id;
		ast::BuiltInType returnType;
		std::vector<ast::BuiltInType> paramTypes;

		// Define operator < for strict weak ordering
		bool operator<(const func& other) const {

			// compare by id first
			if (id != other.id)
				return id < other.id;

			// compare by type if ids are equal
			if (returnType != other.returnType)
				return returnType < other.returnType;

			// compare by paramTypes lexicographically
			return paramTypes < other.paramTypes;
		}

		bool operator==(const func& other) const {
			
			return id == other.id && returnType == other.returnType && paramTypes == other.paramTypes;
		}
	};

	struct var {
		std::string id;
		int offset; // INT_MAX presents an end of a frame

		// Define operator < for strict weak ordering
		bool operator<(const var& other) const {

			// compare by id first
			if (id != other.id)
				return id < other.id;

			// compare by offset
			return offset < other.offset;
		}
	};

    /* SemanticVisitor class
     * This class is used to check the AST for semantic validation.
     */

    class SemanticVisitor : public Visitor {
    private:

    public:
		// symbols_stack maintain variable scope-validity
		std::stack<var> symbols_stack; // {id, offset}
		std::unordered_map<std::string, ast::BuiltInType> symbols_map; // id -> type
		std::stack<int> next_offset_stack;
		std::unordered_map<std::string, int> global_symbol_table;

		// declared functions validity
		std::unordered_map<std::string, func> funcs_map; // string(func_name) -> {id, type, formals}

		ast::BuiltInType curr_func_ret_type;

		output::ScopePrinter printer;

        SemanticVisitor();

		int in_while_loop;

		int max_curr_offset;

		ast::BuiltInType func_return_type;

		void openSymbolFrame();

		void closeSymbolFrame(bool func_frame, int line);

		bool symbolExists(const ast::ID &id);

		void addSymbol(ast::ID &id, const ast::BuiltInType &type, ast::Node &node, bool func_arg);

		void changeSymbolType(const ast::ID &id, const ast::BuiltInType &type, ast::Node &node);

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

#endif // SEMANTIC_HPP
