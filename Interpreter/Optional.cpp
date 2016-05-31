#include "Optional.h"
#include "EvalState.h"

namespace dust {
	Optional::Optional() {}

	Optional::Optional(EvalState& e) : valid{ !e.empty() } {
		if (valid) value = e.pop();
	}

	bool Optional::isValid() const {
		return valid;
	}

	bool Optional::set(EvalState& e) {
		copy(e);								// Copy handles getting the optional value (ie. sets valid and value
		if (valid) e.pop();						// Copy leaves the object on top of the stack so pop it if possible

		return valid;
	}

	bool Optional::set(const Optional& o) {
		value = o.value;

		return valid = o.valid;
	}

	bool Optional::copy(EvalState& e) {
		valid = !e.empty();
		value = valid ? e.at() : impl::Value{0, 0};

		return valid;
	}

	impl::Value Optional::get() const {
		return value;
	}

}