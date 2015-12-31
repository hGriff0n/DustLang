#include "Function.h"
#include "ast.h"

namespace dust {
	namespace impl {

		// Native specialization
		class SysFunction : public FunctionBase {
			private:
				NativeFn fn;

			public:
				SysFunction(const NativeFn& f) : fn{ f } {}

				virtual int call(EvalState& e) {
					return fn(e);
				}
		};

		// Dust specialization
		class DustFunction : public FunctionBase {
			private:
				std::shared_ptr<parse::ASTNode> fn;

			public:
				DustFunction(const std::shared_ptr<parse::ASTNode>& f) : fn{ f } {}

				virtual int call(EvalState& e) {
					fn->eval(e);
					return -1;
				}
		};


		// Function interface
		Function::Function(const std::shared_ptr<parse::ASTNode>& f) : fn{ std::make_shared<DustFunction>(f) } {}
		Function::Function(const NativeFn& f) : fn{ std::make_shared<SysFunction>(f) } {}

		void Function::push(EvalState& e, Value&& val) {
			impl::push(e, val);
		}

		int Function::call(EvalState& e) {
			return fn->call(e);
		}

		int Function::operator()(EvalState& e) {
			return fn->call(e);
		}
	}

}