#pragma once

#include "Value.h"
#include <map>

namespace dust {
	namespace impl {

		class Table {
			public:
				typedef impl::Value key_type;
				typedef impl::Variable val_type;
				typedef std::map<key_type, val_type> storage;

			private:
				Table* parent;
				storage vars;
				size_t next = 1;

			public:
				Table();
				Table(Table* p);

				Variable& getVar(const key_type& key);
				bool has(const key_type& key);
				Value getVal(const key_type& key);

				Variable& getNext();
				size_t size();

				storage::iterator begin();
				storage::iterator end();

				Table* getPar();
				Table* findDef(const key_type& key);
		};

	}

	typedef impl::Table* Table;

}