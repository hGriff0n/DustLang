#include "Table.h"
#include "TypeTraits.h"

#include <algorithm>

namespace dust {
	namespace impl {

		Table::Table() : parent{ nullptr } {}
		Table::Table(Table* p) : parent{ p } {}

		Value Table::getVal(const key_type& key) {
			return getVar(key).val;
		}

		Variable& Table::getVar(const key_type& key) {
			return vars[key];
		}

		Table* Table::findDef(const key_type& key) {
			Table* search = this;

			while (search && !search->hasKey(key))
				search = search->parent;

			return search;
		}

		bool Table::hasKey(const key_type& key) {
			return vars.count(key) > 0;
		}

		bool Table::contains(const impl::Value& val) {
			return std::find_if(begin(), end(), [&](auto pair) {
				return pair.second.val == val;
			}) != end();
		}

		bool Table::okayKey(const Value& t) {
			return true;
		}

		bool Table::okayValue(const Value& t) {
			return true;
		}

		Table::storage::iterator Table::begin() {
			return vars.begin();
		}

		Table::storage::iterator Table::end() {
			return vars.end();
		}

		Table::storage::iterator Table::next(const key_type& key) {
			if (key.type_id == type::Traits<Nil>::id) return vars.begin();

			return ++find(key);
		}

		Table::storage::iterator Table::find(const key_type& key) {
			if (key.type_id == type::Traits<Nil>::id) return vars.end();

			return vars.find(key);
		}

		Table::storage::iterator Table::iend() {
			return std::find_if_not(begin(), end(), [](auto e) {
				return e.first.type_id == type::Traits<int>::id;
			});
		}

		size_t Table::size() {
			return vars.size();
		}
		
		Table* Table::getPar() {
			return parent;
		}

		void Table::setNext(int n) {
			next_val = n;
		}

		int Table::getNext() {
			return next_val++;
		}

		//Variable& Table::getNext() {
		//	return vars[impl::Value{ next_val++, type::Traits<int>::id }];
		//}
	}
}