#pragma once

#include "CallStack.h"
#include "TypeSystem.h"

namespace dust {
	class EvalState : public CallStack {
		private:
			std::map<std::string, impl::Variable> vars;		// Possibly temporary depending on how the global environment is implemented
			impl::TypeSystem ts;
			impl::GC gc;

		protected:
			void forceType(int, size_t);

			// Variables
			void staticTyping(impl::Variable&, bool);
			void newVar(std::string, bool, bool);

		public:
			EvalState() : ts{}, gc{}, CallStack{ gc }, vars{} {}

			// Call functions
			EvalState& call(std::string);
			EvalState& callOp(std::string);				// Temporary methods until I determine how functions and tables will be implemented
			EvalState& callMethod(std::string);

			// Set/Get Variables
			void setVar(std::string, bool = false, bool = false);
			void getVar(std::string);

			// Variable flags
			void mark_constant(std::string);
			void set_typing(std::string, size_t);

			friend void initState(EvalState&);
	};


}