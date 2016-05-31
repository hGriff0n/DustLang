#pragma once

#include <functional>
#include <memory>
#include "TypeTraits.h"

namespace dust {

	// Forward Declarations
	class EvalState;
	using NativeFn = std::function<int(EvalState&)>;

	namespace parse {
		class ASTNode;
	}

	namespace impl {

		// Forward declarations to get this to work
		class GC;
		GC& getGC(EvalState&);
		void push(EvalState&, Value);

		/*
		 * Base class to abstract native/dust distinction
		 * Not extensible in relation to Function (no way to construct a Function from a FunctionBase*)
		 */
		struct FunctionBase {
			virtual int call(EvalState& e) const =0;
		};

		/*
		* C++ implementation of dust functions
		*
		* Using polymorphism to allow for dust and c++ functions under one class interface
		*/
		class Function {
			private:
				std::shared_ptr<FunctionBase> fn;

				template <typename T, typename... Args>
				void push(EvalState& e, T&& val, Args&&... args) const {
					push(e, std::forward<Args>(args)...);
					push(e, std::forward<T>(val));					// first arg on the top
				}

				template <typename T>
				void push(EvalState& e, T&& val) const {
					impl::push(e, type::Traits<T>::make(val, getGC(e)));
				}
				void push(EvalState& e, Value&& v) const;

			public:
				Function(const std::shared_ptr<parse::ASTNode>& f);
				Function(const NativeFn& f);

				// Call the function using the arguments on the stack
				int call(EvalState& e) const;
				int operator()(EvalState& e) const;

				// Call the function using the given arguments
				template <typename... Args>
				int call(EvalState& e, Args&&... args) const {
					push(e, std::forward<Args>(args)...);

					return fn->call(e);
				}
				template <typename... Args>
				int operator()(EvalState& e, Args&&... args) const {
					return call(e, std::forward<Args>(args)...);
				}
		};

	}

	using Function = impl::Function;

}