#pragma once
#include  "Value.h"

namespace dust {

	class EvalState;

	/*
	 * Representation of an optional value (ie. possibly nil)
	 * Mainly sugar for API users to implement some functions
	 */
	class Optional {
		private:
			impl::Value value;
			bool valid = false;

		public:
			Optional();
			Optional(EvalState&);

			bool isValid() const;

			bool set(EvalState&);
			bool set(const Optional&);

			bool copy(EvalState&);

			impl::Value get() const;
	};

}