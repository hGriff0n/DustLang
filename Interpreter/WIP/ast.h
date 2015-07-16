#pragma once

#include <memory>
#include <vector>
#include <stack>
#include <string>

#include "defines.h"

class EvalState;

class ASTNode {
	protected:
		std::vector<std::shared_ptr<ASTNode>> children;				// Should I move this to Expression as that's the only one that really needs "children"
		TokenType token;

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

class Literal : public ASTNode {
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

class BinOp : public ASTNode {				// Might eventually be used for function arguments. (Also relies on "children" but that can be worked out)
	protected:
		std::string oper;

	public:
		BinOp(std::string op) : ASTNode{ TokenType::Operator }, oper{ "_op" + op } {}

		std::string to_string() { return oper; }

		EvalState& eval(EvalState&);
};

EvalState& evaluate(std::shared_ptr<ASTNode>&, EvalState&);

std::string _type(Literal&);
void clear(std::stack<std::shared_ptr<ASTNode>>&);
std::string _typename(ASTNode&);


template <typename T>
T node(std::stack<T>& stack) {
	T ret = stack.top();
	stack.pop();
	return ret;
}