#include "Optional.h"
#include "EvalState.h"

namespace dust {
	Optional::Optional() {}

	Optional::Optional(EvalState& e) : valid{ true } {
		if (!e.empty()) value = e.pop();
	}

	bool Optional::isValid() const {
		return valid;
	}

	bool Optional::set(EvalState& e) {
		copy(e);
		if (valid) e.pop();

		return valid;
	}

	bool Optional::set(const Optional& o) {
		value = o.value;

		return valid = o.valid;
	}

	bool Optional::copy(EvalState& e) {
		if (valid = !e.empty())
			value = e.at();
		else
			value.type_id = type::Traits<Nil>::id;

		return valid;
	}

	impl::Value Optional::get() const {
		return value;
	}

}