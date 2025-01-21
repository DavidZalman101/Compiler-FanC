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

	// VISIT methods

	IrVisitor::IrVisitor() {

		error_div_by_zero = codebuffer.emitString("Error division by zero");

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
		/* %var = add i32 0, <value> */
		std::string reg = codebuffer.freshVar();
		node.reg_name = reg;
		codebuffer.emit("\t" + reg + " = add i32 0, " + std::to_string(node.value));
    }

    void IrVisitor::visit(ast::NumB &node) {
		/* %var = add i32 0, <value> */
		std::string reg = codebuffer.freshVar();
		node.reg_name = reg;
		codebuffer.emit("\t" + reg + " = add i32 0, " + std::to_string(node.value));
    }

    void IrVisitor::visit(ast::String &node) {
    }

    void IrVisitor::visit(ast::Bool &node) {
		/* %var = add i32 <0/1>, <value> */
		std::string reg = codebuffer.freshVar();
		node.reg_name = reg;
		if (node.value)
			codebuffer.emit("\t" + reg + " = add i32 0, " + std::to_string(node.value));
		else
			codebuffer.emit("\t" + reg + " = add i32 0, " + std::to_string(node.value));
    }

    void IrVisitor::visit(ast::ID &node) {
		// if the offset if negative->its a function arg
		// o.w. its a local argument

		int var_offset = node.offset;
		if (var_offset >= 0) {
			/* local argument */
			// always reload the value from the stack to a new reg
			node.reg_name = codebuffer.freshVar();
			std::string ptr = "%t" + std::to_string(var_offset) + "_stack_ptr";
			codebuffer.emit("\t" + node.reg_name + " = load i32, i32* " + ptr);

		} else {
			/* function arguemnt */
			int argument_number = abs(var_offset) - 1;
			node.reg_name = "%" + std::to_string(argument_number);
		}
    }

    void IrVisitor::visit(ast::BinOp &node) {
		node.left->accept(*this);
		node.right->accept(*this);

		std::string left_exp_reg = node.left->reg_name;
		std::string right_exp_reg = node.right->reg_name;
		node.reg_name = codebuffer.freshVar();

		std::string is_zero = codebuffer.freshVar() + "_is_zero";
		std::string zero_block = codebuffer.freshLabel() + "_zero_block";
		std::string non_zero_block = codebuffer.freshLabel() + "_non_zero_block";

		switch(node.op) {
			case(ast::BinOpType::ADD):
				codebuffer.emit("\t" + node.reg_name + " = add i32 " + left_exp_reg + ", " + right_exp_reg);	
				goto check_byte_overflow;
				break;

			case(ast::BinOpType::SUB):
				codebuffer.emit("\t" + node.reg_name + " = sub i32 " + left_exp_reg + ", " + right_exp_reg);
				goto check_byte_overflow;
				break;

			case(ast::BinOpType::MUL):
				codebuffer.emit("\t" + node.reg_name + " = mul i32 " + left_exp_reg + ", " + right_exp_reg);
				goto check_byte_overflow;
				break;

			case(ast::BinOpType::DIV):
				// check if dividing by zero
				codebuffer.emit("\t" + is_zero + " = icmp eq i32 " + right_exp_reg + ", 0");
				codebuffer.emit("\tbr i1 " + is_zero + ", label " + zero_block + ", label " + non_zero_block);

				codebuffer.emit("");

				/* zero block*/
				codebuffer.emitLabel(zero_block);
				// print error msg and exit
				codebuffer.emit("\t; check if dividing by zero");
				//codebuffer.emit("\tcall void @print(i8* " + error_div_by_zero + ")");
				codebuffer << "\tcall i32 (i8*, ...) @printf(i8* getelementptr inbounds ([23 x i8], [23 x i8]* "
				<< error_div_by_zero << ", i32 0, i32 0))" << std::endl;
				codebuffer.emit("\tcall void @exit(i32 0)");	
				codebuffer.emit("\tunreachable");
				codebuffer.emit("");

				
				/* non zero block */
				codebuffer.emitLabel(non_zero_block);


				/* divide */
				codebuffer.emit("\t; divide");
				if (node.exp_type == ast::INT)
					codebuffer.emit("\t" + node.reg_name + " = sdiv i32 " + left_exp_reg + ", " + right_exp_reg);
				else
					codebuffer.emit("\t" + node.reg_name + " = udiv i32 " + left_exp_reg + ", " + right_exp_reg);
			default:

			check_byte_overflow:
				// if the result is a byte, make sure to zero the upper 24 bits
				if (node.exp_type == ast::BYTE) {
					std::string truced_reg = codebuffer.freshVar();
					codebuffer.emit("\t" + truced_reg + " = and i32 " + node.reg_name + ", 255");
					node.reg_name = truced_reg;
				}
				break;
		}
    }

    void IrVisitor::visit(ast::RelOp &node) {

		node.left->accept(*this);
		node.right->accept(*this);

		std::string left_exp_reg = node.left->reg_name;
		std::string right_exp_reg = node.right->reg_name;
		node.reg_name = codebuffer.freshVar();

		switch(node.op) {

			// ==
			case(ast::EQ):
				codebuffer.emit("\t" + node.reg_name + " = icmp eq i32 " + left_exp_reg + ", " + right_exp_reg);
			break;

			// !=
			case(ast::NE):
				codebuffer.emit("\t" + node.reg_name + " = icmp ne i32 " + left_exp_reg + ", " + right_exp_reg);
			break;

			// <
			case(ast::LT):
				codebuffer.emit("\t" + node.reg_name + " = icmp slt i32 " + left_exp_reg + ", " + right_exp_reg);
			break;

			// >
			case(ast::GT):
				codebuffer.emit("\t" + node.reg_name + " = icmp sgt i32 " + left_exp_reg + ", " + right_exp_reg);
			break;

			// <=
			case(ast::LE):
				codebuffer.emit("\t" + node.reg_name + " = icmp sle i32 " + left_exp_reg + ", " + right_exp_reg);
			break;
			break;

			// >=
			case(ast::GE):
				codebuffer.emit("\t" + node.reg_name + " = icmp sge i32 " + left_exp_reg + ", " + right_exp_reg);
			break;

			default:
			break;
		}

		// node.reg is i1, convert it to i32
		std::string extended_reg = codebuffer.freshVar();
		codebuffer.emit("\t" + extended_reg + " = zext i1 " + node.reg_name + " to i32");
		node.reg_name = extended_reg;
    }

    void IrVisitor::visit(ast::Type &node) {
    }

    void IrVisitor::visit(ast::Cast &node) {
    }

    void IrVisitor::visit(ast::Not &node) {
		node.exp->accept(*this);

		node.reg_name = codebuffer.freshVar();
		codebuffer.emit("\t" + node.reg_name + " = xor i32 " + node.exp->reg_name + ", 1");
    }

    void IrVisitor::visit(ast::And &node) {

		node.reg_name = codebuffer.freshVar();
		std::string ptr = codebuffer.freshVar();
		std::string cmp = codebuffer.freshVar();

		std::string check = codebuffer.freshLabel();
		std::string done = codebuffer.freshLabel();

		codebuffer.emit("\t" + ptr + " = alloca i32, i32 1");
		codebuffer.emit("\tstore i32 0, i32* " + ptr);
		node.left->accept(*this);
		codebuffer.emit("\t" + cmp + " = icmp eq i32 " + node.left->reg_name + ", 0");
		codebuffer.emit("\tbr i1 " + cmp + ", label " + done + ", label " + check);

		codebuffer.emitLabel(check);
		node.right->accept(*this);
		codebuffer.emit("\tstore i32 " + node.right->reg_name + ", i32* " + ptr);
		codebuffer.emit("\t br label " + done);

		codebuffer.emitLabel(done);
		codebuffer.emit("\t" + node.reg_name + " = load i32, i32* " + ptr);
    }

    void IrVisitor::visit(ast::Or &node) {

		node.reg_name = codebuffer.freshVar();
		std::string ptr = codebuffer.freshVar();
		std::string cmp = codebuffer.freshVar();
		std::string cmp2 = codebuffer.freshVar();

		std::string label1 = codebuffer.freshLabel();
		std::string label2 = codebuffer.freshLabel();
		std::string done = codebuffer.freshLabel();

		codebuffer.emit("\t" + ptr + " = alloca i32, i32 1");
		codebuffer.emit("\tstore i32 0, i32* " + ptr);
		node.left->accept(*this);
		codebuffer.emit("\t" + cmp + " = icmp eq i32 " + node.left->reg_name + ", 1");
		codebuffer.emit("\tbr i1 " + cmp + ", label " + label1 + ", label " + label2);

		codebuffer.emitLabel(label1);
		codebuffer.emit("\tstore i32 1, i32* " + ptr);
		codebuffer.emit("\tbr label " + done);

		codebuffer.emitLabel(label2);
		node.right->accept(*this);
		codebuffer.emit("\tstore i32 " + node.right->reg_name + ", i32* " + ptr);
		codebuffer.emit("\tbr label " + done);

		codebuffer.emitLabel(done);
		codebuffer.emit("\t" + node.reg_name + " = load i32, i32* " + ptr);
    }

    void IrVisitor::visit(ast::ExpList &node) {
    }

    void IrVisitor::visit(ast::Call &node) {
    }

    void IrVisitor::visit(ast::Statements &node) {
		for (auto it = node.statements.begin(); it != node.statements.end(); ++it) {
			(*it)->accept(*this);
			codebuffer.emit("");
		}
	}

    void IrVisitor::visit(ast::Break &node) {
    }

    void IrVisitor::visit(ast::Continue &node) {
    }

    void IrVisitor::visit(ast::Return &node) {

		if (!node.exp)
			codebuffer.emit("\tret void");
		else {
			node.exp->accept(*this);
			codebuffer.emit("\tret i32 " + node.exp->reg_name);
		}
    }

    void IrVisitor::visit(ast::If &node) {

		node.then->accept(*this);
    }

    void IrVisitor::visit(ast::While &node) {
    }

    void IrVisitor::visit(ast::VarDecl &node) {
		/* store i32 <default/reg>, i32* %t<offset>_stack_ptr */

		node.id->accept(*this);

		std::string var_address = "%t" + std::to_string(node.id->offset) + "_stack_ptr";

		if (node.init_exp) {
			node.init_exp->accept(*this);
			codebuffer.emit("\tstore i32 " + node.init_exp->reg_name + ", i32* " + var_address);
		} else
				codebuffer.emit("\tstore i32 0, i32* " + var_address);
    }

    void IrVisitor::visit(ast::Assign &node) {
		/* store i32 %exp_reg, i32* %t<offset>_stack_ptr */
		node.exp->accept(*this);

		std::string ptr_var = "%t" + std::to_string(node.id->offset) + "_stack_ptr";
		std::string exp_reg = node.exp->reg_name;
		codebuffer.emit("\tstore i32 " + exp_reg + ", i32* " + ptr_var);
    }

    void IrVisitor::visit(ast::Formal &node) {
    }

    void IrVisitor::visit(ast::Formals &node) {
		// TODO: complete
		int i = -1;
		for (auto it = node.formals.begin(); it != node.formals.end(); ++it)
			(*it)->accept(*this);
    }

    void IrVisitor::visit(ast::FuncDecl &node) {
		/*
			At the begining of a function we allocate space
			for the local arguments as if they are i32.
		*/

		// insert function declaration
		std::string s = FuncDecl_Str(node);
		codebuffer.emit(s + " {");
		// {

		// alocate space on the stack for the local variables
		//codebuffer.emit("init_local_vars:");
		codebuffer.emit("\t; init_local_vars:");
		codebuffer.emit("\t%stack_base_size = add i32 " + std::to_string(node.max_offset + 1) + ", 0");
		codebuffer.emit("\t%ptr_stack_base = alloca i32, i32 %stack_base_size");
		codebuffer.emit("\tstore i32 0, i32* %ptr_stack_base");
		codebuffer.emit("");

		// create a pointer reg for each element - format: %t<offset>_stack_ptr
		codebuffer.emit("\t; init pointers to local arguments");
		for (int i = 0; i < node.max_offset + 1; i++)
			codebuffer.emit("\t%t" + std::to_string(i) + "_stack_ptr = getelementptr i32," + 
							"i32* %ptr_stack_base, i32 " + std::to_string(i));

		codebuffer.emit("");

		// TODO: visit the formals
		node.formals->accept(*this);

		// TODO: visit the body
		node.body->accept(*this);
		codebuffer.emit("}\n");
		//}
    }

    void IrVisitor::visit(ast::Funcs &node) {
		// print functions were inserted at construction time

		// travel each declared function
		codebuffer.emit(";functions");
		for (auto it = node.funcs.begin(); it != node.funcs.end(); ++it) {
			(*it)->accept(*this);
		}

    }
}
