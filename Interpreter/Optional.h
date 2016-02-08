#pragma once
#include  "Value.h"

namespace dust {

	class EvalState;

	/*
	 * Representation of an optional value (ie. possibly nil)
	 * Mainly sugar for the user to implement functions
	 */
	class Optional {
		private:
			impl::Value value;
			bool valid = false;

		public:
			Optional();
			Optional(EvalState&);

			bool isValid();

			bool set(EvalState&);
			bool set(const Optional&);

			impl::Value get();
	};

}