#include "ast.h"
#include "state.h"

#include <iostream>

EvalState& evaluate(ASTNode::node_ptr& node, EvalState& state) {
	// What about error handling ??? (Solve this later)
	return node->eval(state);
}

std::string _typename(TokenType token) {
	switch (token) {
		case TokenType::Comment:			// 0
			return "Comment";
		case TokenType::Literal:			// 1
			return "Literal";
		case TokenType::Operator:			// 2
			return "Operator";
		case TokenType::Variable:			// 3
			return "Variable";
		case TokenType::Expr:				// 98
			return "Expression";
		case TokenType::Debug:				// 99
			return "Debug";
		case TokenType::Assignment:			// 100 (tmp)
			return "Assignment";
		case TokenType::List:				// 101 (tmp)
			return "List";
		default:
			return "Unrecognized Token";
	}
}

std::string _typename(ASTNode& node) {
	return _typename(node.token_type());
}

std::string _type(Literal& node) {
	// Or I could have ValType : std::string
	switch (node.value_type()) {
		case ValType::BOOL:					// 0
			return "BOOL";
		case ValType::INT:					// 1
			return "INT";
		case ValType::FLOAT:				// 2
			return "FLOAT";
		case ValType::STRING:				// 3
			return "STRING";
		case ValType::FUNCTION:				// 4
			return "FUNCTION";
		case ValType::TABLE:				// 5
			return "TABLE";
		default:
			return "Unrecognized Literal Type";
	}
}

void clear(stack<ASTNode::node_ptr>& s) {
	while (!s.empty())
		s.pop();
}

std::string Literal::to_string() { return _type(*this) + " " + val; }
std::string Variable::to_string() { return name; }
std::string Assignment::to_string() { return op; }
std::string List::to_string() { return "<" + _typename(vals) + ">"; }

EvalState& ASTNode::eval(EvalState& state) {
	// This evaluates right->left
	for (auto n : *this) 
		n->eval(state);

	return state;
}

EvalState& Literal::eval(EvalState& state) {
	state.push(std::stoi(val));
	return state;
}

EvalState& UnOp::eval(EvalState& state) {
	ASTNode::eval(state).call(this->oper);			// Work on calling convention??
	return state;
}

EvalState& BinOp::eval(EvalState& state) {
	//rhs()->eval(state).push(lhs()->eval(state).pop());	// For some reason this works

	// Change when I change the syntax/semantics for DustFuncs
		// Currently expect: {last_arg}, ..., {first_arg}
		// Might change to: {first_arg}, ..., {last_arg}	// In this case the swap is unnecessary
		// Also change how parsing lays out the data (optimized for the current expectation?)
	rhs()->eval(lhs()->eval(state)).swap().call(oper);		// Call lhs, then rhs, then swap their positions on the stack, then call (operators expect stack = ..., {rhs}, {lhs})

	//{...}.getStack().swap();								// Change EvalState::getStack to return stack&
	//state.call(oper);										// Unless I pop and push intermediate arguments (how maintainable is this though)
	return state;
}

EvalState& Variable::eval(EvalState& state) {
	state.push(state.get(this->name));				// Can I rework this to push the name on the stack (but then how do I get the value?)
	return state;
}


// Modify these two to handle multiple assignment

EvalState& Assignment::eval(EvalState& state) {
	/*/
	auto variable = var();							// Get reference to the variable
	val()->eval(state);								// Push the expression value onto the stack

	if (op != ":") {								// If the assignment is compound
		variable->eval(state);						// Push the variable's value onto the stack
		// val()->eval(variable->eval(state));		// If the syntax is changed as shown in BinOp
		state.call("_op" + op.substr(1));			// Call the linked operator
	} // else
	// val()->eval(state);

	state.set(variable->to_string(), state.top());	// Perform assignment (assuming to_string() == Variable::name)

	return state;
	/*/

	//auto _val = val();
	//val()->eval(state);

	auto r_var = var()->begin(), l_var = var()->end();			// could convert to use std::make_pair
	auto r_val = val()->begin(), l_val = val()->end();
	int var_size = l_var - r_var, val_size = l_val - r_val;
	//std::string last_name = (*r_var)->to_string();

	// Will have to adjust these to account for multiple returns from functions
		// I don't know how many values the function will return at parse-time (some of my assumptions will be invalid)
		// Possibly keep a running tally of "used" variables and values
			// "request" a new value iff there is still a variable to take it (update based on how many were variables pushed on the stack)
			// don't pair the request and assignment loops

	// More values than variables (Readjust the val iterator (Can I store the new iterator???))
		// This causes all values to the right to not be evaluated (I could possibly optimize this by memoizing the iterator)
		// This still might have some difficulties when dealing with multiple returns from functions
			// Could functions increase the number of variables ???
	while (val_size > var_size) {
		++r_val; --val_size;
	}

	// Evaluate expr_list (left->right)
		// Will have to adjust to account for functions with multiple returns
	while (r_val != l_val) {
		--l_val;
		(*l_val)->eval(state);		// take care to 
	}

	// More variables than values (Push nil(0) on the stack)
	while(var_size > val_size) {
		state.push(0); --var_size;
	}

	// Pair the assignments (take care to leave the last value on the stack)
		// There's also the issue of compound assignment (I could have a special compound function variable that performs the extra stuff?)
			// Could do a template dispatch iff I restrict the compound to a single char (no :<=, currently valid)
	while (l_var != r_var) {
		state.set((*r_var)->to_string(), state.pop());
		//std::cout << (*r_var)->to_string() << ": " << state.pop() << std::endl;
		++r_var;
	}

	//state.push(state.get(last_name));
	state.push(state.get((*var()->begin())->to_string()));
	
	//*/

	//state.push(0);

	return state;
}

// List data is stored right->left
EvalState& List::eval(EvalState& state) {
	//*/
	auto n = end();
	while (n != begin())
		(*--n)->eval(state);

	/*/

	for (auto n : *this)
		n->eval(state);
		//std::cout << n->to_string() << " - ";
	//*/

	//std::cout << "s: " << size() << std::endl;
	//state.push(0);

	return state;
}