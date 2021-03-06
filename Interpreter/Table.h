#pragma once

#include "Value.h"
#include <map>

namespace dust {
	namespace impl {

		/*
		 * C++ implementation of dust tables
		 *  This class is also used internally to implement scopes 
		 */
		class Table {
			public:
				typedef Value key_type;
				typedef Variable val_type;
				typedef std::map<key_type, val_type> storage;

			private:
				Table* parent;
				storage vars;
				size_t next_val = 1;

			public:
				Table();
				Table(Table* p);

				// Table Access
				Value getVal(const key_type& key);
				Variable& getVar(const key_type& key);
				Table* findDef(const key_type& key);

				// Table Queries
				bool hasKey(const key_type& key);
				bool contains(const Value& val);
				bool okayKey(const Value& key);
				bool okayValue(const Value& val);

				// Iterators
				storage::iterator begin();
				storage::iterator end();
				storage::iterator iend();				// Array end (needs testing)
				storage::iterator next(const key_type& key);
				storage::iterator find(const key_type& key);

				// Internal State Queries
				size_t size();
				Table* getPar();

				// Array Interactions
				int getNext();
				void setNext(int n);
				//Variable& getNext();
		};
	}

	typedef impl::Table* Table;

}