#include "Optional.h"
#include "EvalState.h"

namespace dust {
	Optional::Optional() {}

	Optional::Optional(EvalState& e) : valid{ true } {
		if (!e.empty()) value = e.pop();
	}

	bool Optional::isValid() {
		return valid;
	}

	bool Optional::set(EvalState& e) {
		if (valid = !e.empty()) value = e.pop();

		return valid;
	}

	bool Optional::set(const Optional& o) {
		value = o.value;

		return valid = o.valid;
	}

	impl::Value Optional::get() {
		if (!valid) value.type_id = type::Traits<Nil>::id;				// Ensure the value is a Nil type if the optional is not valid

		return value;
	}

}