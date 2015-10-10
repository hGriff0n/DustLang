#pragma once

#include "Value.h"
#include <string>
#include <map>

namespace dust {
	namespace impl {

		class Table {
			using key_type = std::string;
			using storage_type = std::map<key_type, Variable>;			// Would an unordered_map be better ?

			private:
				Table* parent;											// only global and outer tables (not sub-tables) have parent == nullptr
				storage_type vars;										// how to implement this though (in regards to tables)
				// std::map<size_t, std::map<Value, Variable>>
				// std::map<Value, Variable>							Simplest to implement, possible conflicts

			protected:
			public:
				Table();
				Table(Table* p);

				Variable& getVar(const key_type& key);
				void associate(const key_type& key, Variable val);
				bool has(const key_type& key);
				Value getVal(const key_type& key);

				Table* getPar();
		};

	}
}