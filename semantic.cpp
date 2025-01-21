#include "semantic.hpp"
#include <iostream>

namespace semantic{

    /* SemanticVisitor implementation */

	std::vector<std::string> paramTypesStrings(std::vector<ast::BuiltInType> types) {

		std::vector<std::string> s_types = {"VOID", "BOOL", "BYTE", "INT", "STRING"};
		std::vector<std::string> res;

		for (ast::BuiltInType type : types)
			res.push_back(s_types[int(type)]);

		return res;
	}

	std::vector<ast::BuiltInType> getParamsTypes(const std::vector<std::shared_ptr<ast::Formal>> &formals) {

		std::vector<ast::BuiltInType> res;

		for (auto it = formals.begin(); it != formals.end(); ++it)
			res.push_back(((*it)->type)->type);

		return res;
	}

	void SemanticVisitor::openSymbolFrame() {

		if (!symbols_stack.empty() && symbols_stack.top().offset != INT_MAX && symbols_stack.top().offset >= 0)
			next_offset_stack.push(symbols_stack.top().offset + 1);
		else if (!symbols_stack.empty() && symbols_stack.top().offset < 0)
			next_offset_stack.push(0);
		else
			next_offset_stack.push(next_offset_stack.top());

		symbols_stack.push({"", INT_MAX});
	}

	void SemanticVisitor::closeSymbolFrame(bool func_frame, int line) {

		while (!symbols_stack.empty() && symbols_stack.top().offset != INT_MAX) {
			symbols_map.erase(symbols_stack.top().id);
			symbols_stack.pop();
		}

		if (!symbols_stack.empty())
			symbols_stack.pop(); // remove {"", VOID, INT_MAX}

		next_offset_stack.pop();
	}

	bool SemanticVisitor::symbolExists(const ast::ID &id) {

		return symbols_map.find(id.value) != symbols_map.end();
	}

	void SemanticVisitor::addSymbol(ast::ID &id, const ast::BuiltInType &type, ast::Node &node, bool func_arg) {

		int offset = 0;

		if (symbolExists(id) || funcs_map.find(id.value) != funcs_map.end())
			output::errorDef(node.line, id.value);

		// function decl argument
		if (func_arg) {
			
			if (symbols_map.empty()) {
				symbols_stack.push({id.value, -1});
				symbols_map[id.value] = type;
				goto emit;
			}

			if (symbolExists(id))
				output::errorDef(node.line, id.value);

			symbols_stack.push({id.value, symbols_stack.top().offset - 1});
			symbols_map[id.value] = type;
			goto emit;
		}

		// if this is the first element in the frame, use the next_offset_stack
		// o.w. your offset is the one before you increased by 1
		if (!(symbols_stack.empty() || symbols_stack.top().offset == INT_MAX || symbols_stack.top().offset < 0))
			offset = symbols_stack.top().offset + 1;
		else
			offset = next_offset_stack.top();

		symbols_stack.push({id.value, offset});
		symbols_map[id.value] = type;
	
		emit:
			id.offset = offset;
			max_curr_offset = std::max(max_curr_offset, id.offset);
			printer.emitVar(id.value, type, symbols_stack.top().offset);
			global_symbol_table[id.value] = offset;
	}

	void SemanticVisitor::changeSymbolType(const ast::ID &id, const ast::BuiltInType &type, ast::Node &node) {

		if (!symbolExists(id))
			output::errorUndef(node.line, id.value);

		symbols_map[id.value] = type;
	}

    SemanticVisitor::SemanticVisitor() {
		in_while_loop = 0;
	}

	// VISIT methods

    void SemanticVisitor::visit(ast::Num &node) {

		node.exp_type = ast::INT;
    }

    void SemanticVisitor::visit(ast::NumB &node) {

		if (node.value > 255)
			output::errorByteTooLarge(node.line, node.value);

		node.exp_type = ast::BYTE;
    }

    void SemanticVisitor::visit(ast::String &node) {

		node.exp_type = ast::STRING;
    }

    void SemanticVisitor::visit(ast::Bool &node) {

		node.exp_type = ast::BOOL;
    }

    void SemanticVisitor::visit(ast::ID &node) {

		// if the ID is a function
		if (symbolExists(node))
			node.exp_type = symbols_map[node.value];
		else if (funcs_map.find(node.value) != funcs_map.end()) {
			node.exp_type = ast::FUNC;
			node.exp_name = node.value;
		}
		else
			output::errorUndef(node.line, node.value);

		node.offset = global_symbol_table[node.value];
    }

    void SemanticVisitor::visit(ast::BinOp &node) {

		node.left->accept(*this);
		node.right->accept(*this);

		// cannot use function id's as vars
		if (node.left->exp_type == ast::FUNC)
			output::errorDefAsFunc(node.line, node.left->exp_name);

		if (node.right->exp_type == ast::FUNC)
			output::errorDefAsFunc(node.line, node.right->exp_name);

		// ops must be numerical variables
		if ((node.left->exp_type != ast::INT && node.left->exp_type != ast::BYTE) ||
			(node.right->exp_type != ast::INT && node.right->exp_type != ast::BYTE))
			output::errorMismatch(node.line);

		// binop type
		if (node.left->exp_type == ast::BYTE && node.right->exp_type == ast::BYTE)
			node.exp_type = ast::BYTE;
		else
			node.exp_type = ast::INT;
    }

    void SemanticVisitor::visit(ast::RelOp &node) {

		node.left->accept(*this);
		node.right->accept(*this);

		if ((node.left->exp_type != ast::INT && node.left->exp_type != ast::BYTE) ||
			(node.right->exp_type != ast::INT && node.right->exp_type != ast::BYTE))
			output::errorMismatch(node.line);

		node.exp_type = ast::BOOL;
    }

    void SemanticVisitor::visit(ast::Type &node) {
    }

    void SemanticVisitor::visit(ast::Cast &node) {

		node.exp->accept(*this);
		node.target_type->accept(*this);

		if ((node.exp->exp_type != ast::INT && node.exp->exp_type != ast::BYTE) ||
			(node.target_type->type != ast::INT && node.target_type->type != ast::BYTE))
			output::errorMismatch(node.line);

		node.exp_type = node.target_type->type;
    }

    void SemanticVisitor::visit(ast::Not &node) {

		node.exp->accept(*this);

		if (node.exp->exp_type != ast::BOOL)
			output::errorMismatch(node.line);

		node.exp_type = ast::BOOL;
    }

    void SemanticVisitor::visit(ast::And &node) {

		node.left->accept(*this);
		node.right->accept(*this);

		if (node.left->exp_type != ast::BOOL || node.right->exp_type != ast::BOOL)	
			output::errorMismatch(node.line);

		node.exp_type = ast::BOOL;
    }

    void SemanticVisitor::visit(ast::Or &node) {

		node.left->accept(*this);
		node.right->accept(*this);

		if (node.left->exp_type != ast::BOOL || node.right->exp_type != ast::BOOL)	
			output::errorMismatch(node.line);

		node.exp_type = ast::BOOL;
    }

    void SemanticVisitor::visit(ast::ExpList &node) {

		for (auto it = node.exps.rbegin(); it != node.exps.rend(); ++it)
			(*it)->accept(*this);
    }

	
    void SemanticVisitor::visit(ast::Call &node) {

		// check if we are trying to use a var as a function
		if (symbols_map.find(node.func_id->value) != symbols_map.end()) {
			output::errorDefAsVar(node.line, node.func_id->value);
		}

		// check if function id exists
		if (funcs_map.find(node.func_id->value) == funcs_map.end()) {
			output::errorUndefFunc(node.line, node.func_id->value);
		}

		std::vector<std::string> s_paramTypes = paramTypesStrings(funcs_map[node.func_id->value].paramTypes);

		node.func_id->accept(*this);
		node.args->accept(*this);
		// parse the args
		// allow INT <- BYTE
		if (node.args->exps.size() != funcs_map[node.func_id->value].paramTypes.size())
    		output::errorPrototypeMismatch(node.line, node.func_id->value, s_paramTypes);

		int i{0};

		for (auto it = node.args->exps.begin(); it != node.args->exps.end(); ++it, i++) {
			if (((*it)->exp_type == funcs_map[node.func_id->value].paramTypes[i]) ||
				(funcs_map[node.func_id->value].paramTypes[i] == ast::INT && (*it)->exp_type == ast::BYTE))
				continue;

    		output::errorPrototypeMismatch(node.line, node.func_id->value, s_paramTypes);
		}

		node.exp_type = funcs_map[node.func_id->value].returnType;
    }

    void SemanticVisitor::visit(ast::Statements &node) {

		for (auto it = node.statements.begin(); it != node.statements.end(); ++it) {

			if ((*it)->block) {
				printer.beginScope();
				openSymbolFrame();
			}

			// ...
			(*it)->accept(*this);

			if ((*it)->block) {
				closeSymbolFrame(false, node.line);
				printer.endScope();
			}
		}
	}

    void SemanticVisitor::visit(ast::Break &node) {

		if (in_while_loop == 0)
			output::errorUnexpectedBreak(node.line);
    }

    void SemanticVisitor::visit(ast::Continue &node) {

		if (in_while_loop == 0)
			output::errorUnexpectedContinue(node.line);
    }

    void SemanticVisitor::visit(ast::Return &node) {

		// No exp
		if (!node.exp) {
			if (curr_func_ret_type == ast::VOID)
				return;

			output::errorMismatch(node.line);
		}

		// exp
		node.exp->accept(*this);

		// allow INT <- BYTE
		if ((node.exp->exp_type == curr_func_ret_type) ||
			(curr_func_ret_type == ast::INT && node.exp->exp_type == ast::BYTE))
			return;

		// tried to use func as var
		if (node.exp->exp_type == ast::FUNC)
			output::errorDefAsFunc(node.line, node.exp->exp_name);

		output::errorMismatch(node.line);
    }

    void SemanticVisitor::visit(ast::If &node) {
		// IF

		printer.beginScope();
		openSymbolFrame();

		if (node.then->block) {
			printer.beginScope();
			openSymbolFrame();
		}

		// ...
		node.condition->accept(*this);

		// Check if the condition type is bool
		if (node.condition->exp_type != ast::BOOL)
			output::errorMismatch(node.condition->line);

		node.then->accept(*this);

		if (node.then->block) {
			closeSymbolFrame(false, node.line);
			printer.endScope();
		}

		closeSymbolFrame(false, node.line);
		printer.endScope();

		// ELSE
		if (!node.otherwise)
			return;

		printer.beginScope();
		openSymbolFrame();

		if (node.otherwise->block) {
				printer.beginScope();
				openSymbolFrame();
		}

		// ...
		node.otherwise->accept(*this);

		if (node.otherwise->block) {
			closeSymbolFrame(false, node.line);
			printer.endScope();
		}

		closeSymbolFrame(false, node.line);
		printer.endScope();
    }

    void SemanticVisitor::visit(ast::While &node) {

		// WHILE
		printer.beginScope();
		openSymbolFrame();
		
		node.condition->accept(*this);

		// Check if the condition type is bool
		if (node.condition->exp_type != ast::BOOL)
			output::errorMismatch(node.condition->line);

		in_while_loop++;

		if (node.body->block) {
			printer.beginScope();
			openSymbolFrame();
		}

		// ...
		node.body->accept(*this);

		if (node.body->block) {
			closeSymbolFrame(false, node.line);
			printer.endScope();
		}

		in_while_loop--;
		closeSymbolFrame(false, node.line);
		printer.endScope();
    }

    void SemanticVisitor::visit(ast::VarDecl &node) {

		node.type->accept(*this);

		// make sure id is not func
		if (funcs_map.find((*(node.id)).value) != funcs_map.end())
			output::errorDefAsFunc(node.line, (*(node.id)).value);
		
		if (node.init_exp) {

			node.init_exp->accept(*this);

			// make sure exp is not func
			if (node.init_exp->exp_type == ast::BuiltInType::FUNC)
				output::errorDefAsFunc(node.line, node.init_exp->exp_name);

			// check types
			// allow INT <- BYTE
			if ((node.type->type != node.init_exp->exp_type) && 
				(!(node.type->type == ast::INT && node.init_exp->exp_type == ast::BYTE)))
				output::errorMismatch(node.line);
		}

		// try to add
		addSymbol(*(node.id), node.type->type, node, false);
		node.id->accept(*this);
		node.id->accept(*this);
    }

    void SemanticVisitor::visit(ast::Assign &node) {

		node.id->accept(*this);
		node.exp->accept(*this);

		// check if trying to use a func id as var
		if (funcs_map.find(node.id->value) != funcs_map.end())
			output::errorDefAsFunc(node.line, node.id->value);

		// check if exp is a func
		if (node.exp && node.exp->exp_type == ast::FUNC)
			output::errorDefAsFunc(node.line, node.exp->exp_name);

		// check if var exists
		if (!symbolExists(*(node.id)))
			output::errorUndef(node.line, node.id->value);

		// var exists
		// allow INT <- BYTE
		if ((symbols_map[node.id->value] == node.exp->exp_type) ||
			(symbols_map[node.id->value] == ast::INT && node.exp->exp_type == ast::BYTE))
			return;

		output::errorMismatch(node.line);
    }

    void SemanticVisitor::visit(ast::Formal &node) {

		addSymbol(*node.id, node.type->type, node, true);
    }

    void SemanticVisitor::visit(ast::Formals &node) {

		for (auto it = node.formals.begin(); it != node.formals.end(); ++it)
			(*it)->accept(*this);
    }

    void SemanticVisitor::visit(ast::FuncDecl &node) {

		max_curr_offset = 0;
		global_symbol_table = std::unordered_map<std::string, int>();

		printer.beginScope();
		openSymbolFrame();

		printer.emitFunc(node.id->value,
						 (node).return_type->type,
						 getParamsTypes((node).formals->formals));

		node.id->accept(*this);
		node.return_type->accept(*this);
		node.formals->accept(*this);
		node.body->accept(*this);

		node.max_offset = max_curr_offset;
		max_curr_offset = 0;

		closeSymbolFrame(true, node.line);
		printer.endScope();

		node.symbol_table = global_symbol_table;
    }

    void SemanticVisitor::visit(ast::Funcs &node) {

		// insert print & printi
		printer.emitFunc("print", ast::BuiltInType::VOID, {ast::BuiltInType::STRING});
		funcs_map["print"] = {"print", ast::BuiltInType::VOID, {ast::BuiltInType::STRING}};

		printer.emitFunc("printi", ast::BuiltInType::VOID, {ast::BuiltInType::INT});
		funcs_map["printi"] = {"printi", ast::BuiltInType::VOID, {ast::BuiltInType::INT}};

		// insert all declared functions
		for (auto it = node.funcs.begin(); it != node.funcs.end(); ++it) {

			func new_function = {(*it)->id->value,
								 (*it)->return_type->type,
								 getParamsTypes((*it)->formals->formals)};

			// check if function was already declared
			if (funcs_map.find((*it)->id->value) != funcs_map.end())
				output::errorDef((*it)->id->line, (*it)->id->value);
				//output::errorDef(node.line, (*it)->id->value);

			funcs_map[(*it)->id->value] = new_function;
		}

		// check if "void main()" exists
		func expected_main = {"main", ast::BuiltInType::VOID, {}};
		if (funcs_map.find("main") == funcs_map.end() ||
			!(funcs_map["main"] == expected_main))
			output::errorMainMissing();

		next_offset_stack.push(0);

		// travel each funcDecl
		for (auto it = node.funcs.begin(); it != node.funcs.end(); ++it) {

			curr_func_ret_type = (*it)->return_type->type;
			(*it)->accept(*this);
		}
    }
}
