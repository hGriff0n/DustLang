#include "Table.h"

namespace dust {
	namespace impl {

		Table::Table() : parent{ nullptr } {}
		Table::Table(Table* p) : parent{ p } {}

		void Table::associate(const key_type& key, impl::Variable val) {
			vars[key] = val;
		}

		bool Table::has(const key_type& key) {
			return vars.count(key);
		}

		Variable& Table::getVar(const key_type& key) {
			return vars[key];
		}

		Value Table::getVal(const key_type& key) {
			return getVar(key).val;
		}

		Table* Table::getPar() {
			return parent;
		}
	}
}