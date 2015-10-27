#include "Table.h"

namespace dust {
	namespace impl {

		Table::Table() : parent{ nullptr } {}
		Table::Table(Table* p) : parent{ p } {}
		
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

		Table* Table::findDef(const key_type& key) {
			Table* search = this;

			while (search && !search->has(key)) search = search->parent;

			return search;
		}
	}
}