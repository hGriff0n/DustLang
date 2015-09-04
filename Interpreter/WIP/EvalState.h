#pragma once

#include "CallStack.h"
#include "TypeSystem.h"

namespace dust {
	class EvalState : public CallStack {
		private:
			std::map<std::string, impl::Variable> vars;
			impl::TypeSystem ts;
			impl::GC gc;

		protected:
			void forceType(int, size_t);

		public:
			EvalState() : ts{}, gc{}, CallStack{ gc } {}

			EvalState& call(std::string);
			EvalState& callOp(std::string);				// Temporary methods until I determine how functions and tables will be implemented
			EvalState& callMethod(std::string);
			friend void initState(EvalState&);
	};


}