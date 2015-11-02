#include "Table.h"
#include "TypeTraits.h"

namespace dust {
	namespace impl {

		Table::Table() : parent{ nullptr } {}
		Table::Table(Table* p) : parent{ p } {}

		bool Table::has(const key_type& key) {
			return vars.count(key) > 0;
		}

		Variable& Table::getVar(const key_type& key) {
			return vars[key];
		}

		Value Table::getVal(const key_type& key) {
			return getVar(key).val;
		}

		Variable& Table::getNext() {
			return vars[impl::Value{ next++, type::Traits<int>::id }];
		}

		size_t Table::size() {
			return vars.size();
		}

		Table::storage::iterator Table::begin() {
			return vars.begin();
		}

		Table::storage::iterator Table::end() {
			return vars.end();
		}

		Table* Table::getPar() {
			return parent;
		}

		Table* Table::findDef(const key_type& key) {
			Table* search = this;

			while (search && !search->has(key))
				search = search->parent;

			return search;
		}

	}
}