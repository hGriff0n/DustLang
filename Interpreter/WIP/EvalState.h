#pragma once

#include "CallStack.h"
#include "TypeSystem.h"

namespace dust {
	class EvalState : public CallStack {
		private:
			impl::TypeSystem ts;
			impl::GC gc;

		protected:
			void forceType(int, size_t);

		public:
			EvalState() : ts{}, gc{}, CallStack{ gc } {}

			//EvalState& callOp(std::string);		// Allow immediate access of computation results
			void call(std::string);
			void callOp(std::string);				// Temporary methods until I determine how functions and tables will be implemented
			void callMethod(std::string);
			friend void initState(EvalState&);
	};


}