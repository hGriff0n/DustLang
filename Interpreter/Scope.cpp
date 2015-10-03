#include "Scope.h"

namespace dust {
	namespace impl {

		Scope::Scope() {}

		void Scope::associate(const key_type& key, impl::Variable val) {
			vars[key] = val;
		}

		bool Scope::has(const key_type& key) {
			return vars.count(key);
		}

		Variable& Scope::getVar(const key_type& key) {
			return vars[key];
		}

		Value Scope::getVal(const key_type& key) {
			return getVar(key).val;
		}
	}
}