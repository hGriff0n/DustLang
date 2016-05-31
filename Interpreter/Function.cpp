#include "Function.h"
#include "ast.h"

namespace dust {
	namespace impl {

		// Specialization for C++ functions
		class SysFunction : public FunctionBase {
			private:
				NativeFn fn;

			public:
				SysFunction(const NativeFn& f) : fn{ f } {}

				virtual int call(EvalState& e) const {
					return fn(e);
				}
		};

		// Specialization for Dust functions
		class DustFunction : public FunctionBase {
			private:
				std::shared_ptr<parse::ASTNode> fn;

			public:
				DustFunction(const std::shared_ptr<parse::ASTNode>& f) : fn{ f } {}

				virtual int call(EvalState& e) const {
					fn->eval(e);
					return -1;
				}
		};

		// Function's constructor handles creating the correct polymorphic type based on it's arguments
		Function::Function(const std::shared_ptr<parse::ASTNode>& f) : fn{ std::make_shared<DustFunction>(f) } {}
		Function::Function(const NativeFn& f) : fn{ std::make_shared<SysFunction>(f) } {}

		void Function::push(EvalState& e, Value&& val) const {
			impl::push(e, val);
		}

		int Function::call(EvalState& e) const {
			return fn->call(e);
		}

		int Function::operator()(EvalState& e) const {
			return fn->call(e);
		}
	}

}