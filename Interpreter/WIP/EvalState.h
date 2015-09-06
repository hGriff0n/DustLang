#pragma once

#include "CallStack.h"
#include "TypeSystem.h"

namespace dust {

	class EvalState : public impl::CallStack {
		private:
			std::map<std::string, impl::Variable> vars;		// Possibly temporary depending on how the global environment is implemented
			type::TypeSystem ts;
			impl::GC gc;

		protected:
			void forceType(int, size_t);

			// Variables
			void staticTyping(impl::Variable&, bool);
			void newVar(std::string, bool, bool);

		public:
			EvalState() : ts{}, gc{}, CallStack{ gc }, vars{} {}

			// Call functions
			EvalState& call(std::string fn);
			EvalState& callOp(std::string fn);				// Temporary methods until I determine how functions and tables will be implemented
			EvalState& callMethod(std::string fn);

			// Set/Get Variables
			void setVar(std::string name, bool isConst = false, bool isTyped = false);
			void getVar(std::string var);

			// Variable flags
			// Sets var to const if not const and vice versa
			void mark_constant(std::string var);
			void set_typing(std::string var, size_t typ);
			bool isConst(std::string var);
			bool isStatic(std::string var);

			friend void initState(EvalState&);
	};


}