#pragma once

#include "CallStack.h"
#include "TypeSystem.h"

namespace dust {
	class EvalState : public CallStack {
		private:
			impl::TypeSystem ts;
			impl::GC gc;

		protected:
		public:
			EvalState() : ts{}, gc{}, CallStack{ gc } {}

			void call(std::string);
			friend void initState(EvalState&);
	};


}