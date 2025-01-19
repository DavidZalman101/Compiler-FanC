#include "ir.hpp"
#include <iostream>

namespace Ir{

    /* IrVisitor implementation */

	std::vector<std::string> ir_BuiltInType  = {"i32", "i1", "i8", "i32", "i32", "i32"};

	std::string FuncDecl_Str(const ast::FuncDecl &node) {

		std::string s = "define " + ir_BuiltInType[node.return_type->type] + " @" + node.id->value + "(";

		// add arguments types
		for (auto it = node.formals->formals.begin(); it != node.formals->formals.end(); ++it)
			s += ir_BuiltInType[(*it)->type->type] + ", ";

		if (s.back() == ' ')
			s.pop_back();

		if (s.back() == ',')
			s.pop_back();

		s += ")";

		return s;

	}

	//void IrVisitor::allocate_arguments_on_stack(ast::Formals &node) {
	//	// %ptr = alloca i32
	//	codebuffer.emit("allocate_local_arguments:");

	//	for (auto it = node.formals.begin(); it != node.formals.end(); ++it) {
	//		std::string ptr_name = "stack_ptr_" + (*it)->id->value;
	//		codebuffer.emit("\t" + ptr_name + " = alloca i32");
	//	}
	//}

	// VISIT methods

	IrVisitor::IrVisitor() {

		// read the print functions
		codebuffer<<"; print functions"<<std::endl;
		std::ifstream inputFile("print_functions.llvm");
		if (!inputFile.is_open()) {
			std::cerr<<"Error: Could not open the file."<<std::endl;
			exit(0);
		}

    	std::string line;
    	while (std::getline(inputFile, line))
        	codebuffer<<line<<std::endl;

		inputFile.close();

		codebuffer<<std::endl;
	}

    void IrVisitor::visit(ast::Num &node) {
    }

    void IrVisitor::visit(ast::NumB &node) {
    }

    void IrVisitor::visit(ast::String &node) {
    }

    void IrVisitor::visit(ast::Bool &node) {
    }

    void IrVisitor::visit(ast::ID &node) {
    }

    void IrVisitor::visit(ast::BinOp &node) {
    }

    void IrVisitor::visit(ast::RelOp &node) {
    }

    void IrVisitor::visit(ast::Type &node) {
    }

    void IrVisitor::visit(ast::Cast &node) {
    }

    void IrVisitor::visit(ast::Not &node) {
    }

    void IrVisitor::visit(ast::And &node) {
    }

    void IrVisitor::visit(ast::Or &node) {
    }

    void IrVisitor::visit(ast::ExpList &node) {
    }

    void IrVisitor::visit(ast::Call &node) {
    }

    void IrVisitor::visit(ast::Statements &node) {
	}

    void IrVisitor::visit(ast::Break &node) {
    }

    void IrVisitor::visit(ast::Continue &node) {
    }

    void IrVisitor::visit(ast::Return &node) {
    }

    void IrVisitor::visit(ast::If &node) {
    }

    void IrVisitor::visit(ast::While &node) {
    }

    void IrVisitor::visit(ast::VarDecl &node) {
    }

    void IrVisitor::visit(ast::Assign &node) {
    }

    void IrVisitor::visit(ast::Formal &node) {
    }

    void IrVisitor::visit(ast::Formals &node) {
    }

    void IrVisitor::visit(ast::FuncDecl &node) {
		/*
			At the begining of a function we allocate space
			for the local arguments as if they are i32.
		*/


		// insert function declaration
		std::string s = FuncDecl_Str(node);
		codebuffer.emit(s);
		// {
		codebuffer.emit("{");

		// alocate space on the stack for the local variables
		codebuffer.emit("init_local_vars:");
		codebuffer.emit("\t%stack_base_size = add i32 " + std::to_string(node.max_offset + 1) + ", 0");
		codebuffer.emit("\t%ptr_stack_base = alloca i32, i32 %stack_base_size");
		codebuffer.emit("\tstore i32 0, i32* %ptr_stack_base");
		//allocate_arguments_on_stack(*node.formals);

		// TODO: return dynamically
		codebuffer.emit("\tret i32 0");
		codebuffer.emit("}\n");
		//}


    }

    void IrVisitor::visit(ast::Funcs &node) {
		// print functions were inserted at construction time

		// travel each declared function
		codebuffer.emit(";functions");
		for (auto it = node.funcs.begin(); it != node.funcs.end(); ++it)
			(*it)->accept(*this);

    }
}
