#pragma once

#include <memory>
#include <vector>
#include <stack>
#include <string>

#include "defines.h"

class EvalState;

class ASTNode {
	protected:
		std::vector<std::shared_ptr<ASTNode>> children;				// Should I move this to Expression as that's the only one that really needs it
		TokenType token;
		//Debug data

	public:
		ASTNode(TokenType t) : token{ t } {

		}

		ASTNode& addChild(std::shared_ptr<ASTNode>& n) {
			children.emplace_back(n);
			return *this;
		}

		auto begin() {
			return children.begin();
		}
		auto end() {
			return children.end();
		}

		TokenType token_type() {
			return token;
		}
		virtual std::string to_string() {
			return "";
		}

		virtual EvalState& eval(EvalState&);
};

class Literal : public ASTNode {			// A literal can't have children (except for maybe Tables and functions, but I could make another class in the heirarchy)	
	protected:
		std::string val;
		ValType type;

	public:
		Literal(std::string v, ValType t) : ASTNode{ TokenType::Literal }, val{ v }, type{ t } {};

		ValType value_type() { return type; }
		std::string to_string();

		EvalState& eval(EvalState&);
};

class Expression : public ASTNode {
	protected:
	public:
		Expression() : ASTNode{ TokenType::Expr } {}
};

class UnOp : public ASTNode {
	protected:
		std::string oper;

	public:
		UnOp(std::string op) : ASTNode{ TokenType::Operator }, oper{ "_ou" + op } {}		// _ou for unary operators ???

		std::string to_string() { return oper; }
		EvalState& eval(EvalState&);
};

class BinOp : public ASTNode {				// Might eventually be used for function arguments
	protected:
		std::string oper;

	public:
		BinOp(std::string op) : ASTNode{ TokenType::Operator }, oper{ "_op" + op } {}

		std::string to_string() { return oper; }

		EvalState& eval(EvalState&);
};

EvalState& evaluate(std::shared_ptr<ASTNode>&, EvalState&);		// The return and arguments will change over time (should I return the eval state ???)

std::string _type(Literal&);
void clear(std::stack<std::shared_ptr<ASTNode>>&);
std::string _typename(ASTNode&);

template <typename T, typename... C>
auto makeNode(C&&... args) {
	return std::make_shared<T>(args...);
}

template <typename T>
T node(std::stack<T>& stack) {
	T ret = stack.top();
	stack.pop();
	return ret;
}

/*
For parsing
	Keep a stack of ASTNodes
	The bottom levels of expressions will construct their trees and then push them onto the stack
	Upper levels then would naturally depend on these lower levels
	Also the only nodes on the stack should be either children nodes currently being used to construct
		A section of the tree, or brother nodes waiting for their siblings to "mature"
	Then Upper levels then can be constructed by popping their children nodes from the stack
	The combination is then pushed back onto the stack
	This approach allows me to 'ignore' irrelevant syntax (such as '()' used to group expressions)
	The approach also follows the natural execution of the parser (ie. it matches sub-expressions first)
		The approach is basically a "shift-reduce"

	Also it should be possible for me to "seperate" various sections of parsing (ie. the parser doesn't parse function bodies, it just recognizes that the body exists)
		I might have already typed this elsewhere, but it could simplify lazy evaluation
		This is also a wy to "enable" closures
			The values of closures won't be known until execution time and if the closure is not constructed until then, then the value can be spliced in for the closure during parsing (once and done)
			Alternatively, it's also possible to replace the closures for their values using an already constructed tree (single responsibility)
			The first option simplifies/improves evaluation (how much), the second simplifies parsing
				The first might also make it easier to implement various features such as lazy-evaluation
					It should be noted how lazy evaluation is generally implemented

	I should mention somewhere in dust that all strings are long string literals like Lua's [[]]
		Though some degree of "escaping" would be nice
*/

/*
The current Literal definition requires a step to "dereference" the string during evaluation
	It could be possible to instead perform this "dereference" during parsing
	Even with the current definition, there are multiple ways of "dereferncing"

template <typename T>
class Literal : public ASTNode {
	protected:
		T val;
		ValType type;

	public:
		Literal(T v) : ASTNode{}, val{ v } {};

		std::string tostring() {
			return std::to_string(val);
		}

		// this isn't actually needed here
		void dereference(EvalState& s) {

		}
};

The "double" action then would change to
	ast.push(std::make_shared<Literal<double>>(std::stod(in.string())));
*/