#include "ir.hpp"
#include <iostream>

namespace Ir{

    /* IrVisitor implementation */

	std::vector<std::string> ir_BuiltInType  = {"void", "i32", "i32", "i32", "i32", "i32"};

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

		std::string print_functions =
										"declare i32 @scanf(i8*, ...)\n"
										"declare i32 @printf(i8*, ...)\n"
										"declare void @exit(i32)\n"
										"@.int_specifier_scan = constant [3 x i8] c\"%d\\00\"\n"
										"@.int_specifier = constant [4 x i8] c\"%d\\0A\\00\"\n"
										"@.str_specifier = constant [4 x i8] c\"%s\\0A\\00\"\n"
										"\n"
										"define i32 @readi(i32) {\n"
										"    %ret_val = alloca i32\n"
										"    %spec_ptr = getelementptr [3 x i8], [3 x i8]* @.int_specifier_scan, i32 0, i32 0\n"
										"    call i32 (i8*, ...) @scanf(i8* %spec_ptr, i32* %ret_val)\n"
										"    %val = load i32, i32* %ret_val\n"
										"    ret i32 %val\n"
										"}\n"
										"\n"
										"define void @printi(i32) {\n"
										"    %spec_ptr = getelementptr [4 x i8], [4 x i8]* @.int_specifier, i32 0, i32 0\n"
										"    call i32 (i8*, ...) @printf(i8* %spec_ptr, i32 %0)\n"
										"    ret void\n"
										"}\n"
										"\n"
										"define void @print(i8*) {\n"
										"    %spec_ptr = getelementptr [4 x i8], [4 x i8]* @.str_specifier, i32 0, i32 0\n"
										"    call i32 (i8*, ...) @printf(i8* %spec_ptr, i8* %0)\n"
										"    ret void\n"
										"}\n";

		// read the print functions
		//codebuffer<<"; print functions"<<std::endl;
		//std::ifstream inputFile("print_functions.llvm");
		//if (!inputFile.is_open()) {
		//	std::cerr<<"Error: Could not open the file."<<std::endl;
		//	exit(0);
		//}

    	//std::string line;
    	//while (std::getline(inputFile, line))
        //	codebuffer<<line<<std::endl;

		//inputFile.close();

		codebuffer<<print_functions<<std::endl;
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
		//std::cout<<node.value<<" "<<node.value.size()<<std::endl;
		//node.constant_str = codebuffer.emitString(node.value);
		node.reg_name = codebuffer.emitString(node.value);
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

		if (func_arg_num.count(node.value) != 0) {
			// function argument
			// function argument dont get writtern into them
			// therfore there is no need to manage him, feel free to read his
			// dedicate reg :)
			node.reg_name = func_arg_num[node.value];
		} else {
			int var_offset = node.offset;
			// local argument
			// always reload the value from the stack to a new reg
			node.reg_name = codebuffer.freshVar();
			std::string ptr = "%t" + std::to_string(var_offset) + "_stack_ptr";
			codebuffer.emit("\t" + node.reg_name + " = load i32, i32* " + ptr);

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
		std::string err_zero_div_ptr = codebuffer.freshVar();

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

				codebuffer.emit("\t" + err_zero_div_ptr + " = getelementptr [23 x i8], [23 x i8]* " + error_div_by_zero +
				", i32 0, i32 0");
				codebuffer.emit("\tcall void @print(i8* " + err_zero_div_ptr + ")");

				//codebuffer << "\tcall i32 (i8*, ...) @printf(i8* getelementptr inbounds ([23 x i8], [23 x i8]* "
				//<< error_div_by_zero << ", i32 0, i32 0))" << std::endl;

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
		for (auto it = node.exps.rbegin(); it != node.exps.rend(); ++it)
			(*it)->accept(*this);
    }

    void IrVisitor::visit(ast::Call &node) {

		node.args->accept(*this);

		// take care of print - needs special treatment
		if (node.func_id->value == "print") {
			int len = node.args->exps[0]->str_val.size() + 1;
			//std::string type_str = "[" + std::to_string(len) + " x i8], [" + std::to_string(len) + " x i8]* ";
        	//codebuffer << "\tcall i32 (i8*, ...) @printf(i8* getelementptr inbounds (" + type_str
			//<< node.args->exps[0]->constant_str << ", i32 0, i32 0))" << std::endl;
			//codebuffer << "call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([14 x i8], [14 x i8]* "
			//<< node.args->exps[0]->reg_name << ", i32 0, i32 0))" << std::endl;
			std::string ptr_str = codebuffer.freshVar();
			codebuffer.emit("\t" + ptr_str + " = getelementptr [" + std::to_string(len) + " x i8], [" + 
			std::to_string(len) + " x i8]* " + node.args->exps[0]->reg_name + ", i32 0, i32 0");
			codebuffer.emit("\tcall void @print(i8* " + ptr_str + ")");
			return;
		}

		node.reg_name = codebuffer.freshVar();
		std::string call_str = "\t";
		if (func_retType[node.func_id->value] == "i32")
			call_str += node.reg_name + " = call " + func_retType[node.func_id->value];
		else
			call_str += " call " + func_retType[node.func_id->value];

		call_str += " @" + node.func_id->value + "(";

		if (node.args->exps.size() > 0)
			call_str += "i32 " + node.args->exps[0]->reg_name;

		for (int i = 1; i < node.args->exps.size(); i++)
			call_str += ", i32 " + node.args->exps[i]->reg_name;

		call_str += ")";

		codebuffer.emit(call_str);
    }

    void IrVisitor::visit(ast::Statements &node) {
		for (auto it = node.statements.begin(); it != node.statements.end(); ++it) {
			(*it)->accept(*this);
			codebuffer.emit("");
		}
	}

    void IrVisitor::visit(ast::Break &node) {
		codebuffer.emit("\tbr label " + while_exit_labels.top());
    }

    void IrVisitor::visit(ast::Continue &node) {
		codebuffer.emit("\tbr label " + while_cond_labels.top());
    }

    void IrVisitor::visit(ast::Return &node) {

		had_ret = true;
	
		if (!node.exp)
			codebuffer.emit("\tret void");
		else {
			node.exp->accept(*this);
			codebuffer.emit("\tret i32 " + node.exp->reg_name);
		}
    }

    void IrVisitor::visit(ast::If &node) {

		std::string cmp = codebuffer.freshLabel();

		std::string if_body = codebuffer.freshLabel();
		std::string else_body = codebuffer.freshLabel();
		std::string done = codebuffer.freshLabel();

		node.condition->accept(*this);

		codebuffer.emit("\t" + cmp + " = icmp eq i32 " + node.condition->reg_name + ", 1");
		codebuffer.emit("\tbr i1 " + cmp + ", label " + if_body + ", label " + else_body);

		codebuffer.emitLabel(if_body);
		node.then->accept(*this);
		codebuffer.emit("\tbr label " + done);

		codebuffer.emitLabel(else_body);
		if (node.otherwise)
			node.otherwise->accept(*this);
		codebuffer.emit("\tbr label " + done);

		codebuffer.emitLabel(done);

    }

    void IrVisitor::visit(ast::While &node) {

		std::string cmp = codebuffer.freshVar();

		std::string cond_entrie = codebuffer.freshLabel();
		std::string loop_entrie = codebuffer.freshLabel();
		std::string done = codebuffer.freshLabel();

		while_exit_labels.push(done);
		while_cond_labels.push(cond_entrie);

		codebuffer.emit("\tbr label " + cond_entrie);
		codebuffer.emitLabel(cond_entrie);
		node.condition->accept(*this);
		codebuffer.emit("\t" + cmp + " = icmp eq i32 " + node.condition->reg_name + ", 1");
		codebuffer.emit("\tbr i1 " + cmp + ", label " + loop_entrie + ", label " + done);

		codebuffer.emitLabel(loop_entrie);
		node.body->accept(*this);
		codebuffer.emit("\tbr label " + cond_entrie);

		codebuffer.emitLabel(done);
		while_exit_labels.pop();
		while_cond_labels.pop();
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
		for (auto it = node.formals.begin(); it != node.formals.end(); ++it)
			(*it)->accept(*this);
    }

    void IrVisitor::visit(ast::FuncDecl &node) {
		/*
			At the begining of a function we allocate space
			for the local arguments as if they are i32.
		*/
		int i = 0;
		had_ret = false;
		func_arg_num = std::unordered_map<std::string, std::string>();
		for (auto it = node.formals->formals.begin(); it != node.formals->formals.end(); ++it)
			func_arg_num[(*it)->id->value] = "%" + std::to_string(i++);

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

		if (!had_ret) {
			// defualt ret value
			if (node.return_type->type != ast::VOID)
				codebuffer.emit("\tret i32 0");
			else
				codebuffer.emit("\tret void");
		}
	

		codebuffer.emit("}\n");
		//}
    }

    void IrVisitor::visit(ast::Funcs &node) {
		// print functions were inserted at construction time

		for (auto it = node.funcs.begin(); it != node.funcs.end(); ++it)
			func_retType[(*it)->id->value] = (*it)->return_type->type == ast::VOID ? "void" : "i32";

		func_retType["readi"] = "i32";
		func_retType["print"] = func_retType["printi"] = "void";
			
		// travel each declared function

		codebuffer.emit(";functions");
		for (auto it = node.funcs.begin(); it != node.funcs.end(); ++it) {
			(*it)->accept(*this);
		}

    }
}
