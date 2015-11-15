#pragma once

#include "Value.h"
#include <map>

namespace dust {
	namespace impl {

		/*
		 * C++ Implementation of dust table's
		 *  This class is also used internally to implement scopes 
		 */
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

				// Table Access functions
				Value getVal(const key_type& key);
				Variable& getVar(const key_type& key);
				Table* findDef(const key_type& key);
				bool hasKey(const key_type& key);
				bool contains(const impl::Value& val);


				// Iterators
				storage::iterator begin();
				// End of array iteration
				storage::iterator iend();				// Needs testing
				storage::iterator end();


				// Internal Detail Operations
				size_t size();
				Table* getPar();


				// Array Interaction
				int getNext();
				void setNext(int n);
				//Variable& getNext();
		};
	}

	typedef impl::Table* Table;

}