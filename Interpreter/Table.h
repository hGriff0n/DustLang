#pragma once

#include "Value.h"
#include <string>
#include <unordered_map>

namespace dust {
	namespace impl {

		/*
		 * Class that mimics the semantics of Dust tables
		 * Is also (currently) used to implement scopes
		 */
		class Table {
			using key_type = std::string;
			using storage_type = std::unordered_map<key_type, Variable>;			// Would an unordered_map be better ?

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
				bool has(const key_type& key);
				Value getVal(const key_type& key);

				Table* getPar();
				Table* findDef(const key_type& key);
		};

		// class Scope : public Table;									// All internal operations work on Scope. API calls and storage use Table?

	}

	typedef impl::Table* Table;
}