#pragma once

#include <memory>
#include <string>

#include "stack.h"
#include "defines.h"

class EvalState;

// Run a fine-grained finger over these classes and ensure they only have what they need
class ASTNode {
	public:
		typedef std::shared_ptr<ASTNode> node_ptr;

	protected:
		std::vector<node_ptr> children;				// Should I move this to Expression as that's the only one that really needs it
		TokenType token;
		//Debug data

	public:
		ASTNode(TokenType t) : token{ t } {

		}

		ASTNode& addChild(node_ptr& n) {
			children.emplace_back(n);
			return *this;
		}

		auto begin() {
			return children.begin();
		}
		auto end() {
			return children.end();
		}

		node_ptr get(int i) {
			return children[i];
		}

		TokenType token_type() {
			return token;
		}
		void set_token(TokenType t) {
			token = t;
		}
		virtual std::string to_string() {
			return "";
		}

		virtual EvalState& eval(EvalState&);
};

class Debug : public ASTNode {
	protected:
		std::string msg;

	public:
		Debug(std::string m) : ASTNode{ TokenType::Debug }, msg{ m } {};

		std::string to_string() { return msg; }
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

// change to Function ???
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
		node_ptr lhs() { return get(1); }	// 0 if lhs is first
		node_ptr rhs() { return get(0); }

	public:
		BinOp(std::string op) : ASTNode{ TokenType::Operator }, oper{ "_op" + op } {}

		std::string to_string() { return oper; }

		EvalState& eval(EvalState&);
};

class Variable : public ASTNode {
	protected:
		std::string name;

	public:
		Variable(std::string n) : ASTNode{ TokenType::Variable }, name{ n } {}

		std::string to_string();

		EvalState& eval(EvalState&);
};

// change to Function ???
class Assignment : public ASTNode {
	protected:
		std::string op;
		bool compound;

		// Should I move these to be member variables ???
		node_ptr var() { return ASTNode::get(1); }	// 0 if lhs is first
		node_ptr val() { return ASTNode::get(0); }

	public:
		Assignment(std::string assign_t) : ASTNode{ TokenType::Assignment }, op{ assign_t.substr(1) }, compound{ assign_t != ":" } {}

		std::string to_string();
		EvalState& eval(EvalState&);
};

//template <class N>
class List : public ASTNode {
	protected:
		TokenType vals;

	public:
		List(TokenType t) : ASTNode{ TokenType::List }, vals{ t } {}

		std::string to_string();
		EvalState& eval(EvalState&);

		int size() { return ASTNode::end() - ASTNode::begin(); }
};


EvalState& evaluate(ASTNode::node_ptr&, EvalState&);		// The return and arguments will change over time (should I return the eval state ???)

std::string _type(ValType);
std::string _type(Literal&);
void clear(stack<ASTNode::node_ptr>&);
std::string _typename(TokenType);
std::string _typename(ASTNode&);

template <typename T, typename... C>
auto makeNode(C&&... args) {
	return std::make_shared<T>(args...);
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