#pragma once

#include "RuntimeStorage.h"
#include "Table.h"

namespace dust {
	namespace impl {

		/*
		* RuntimeStorage class specialized for storing dust::Table (through std::pair)
		*/
		class TableStorage : public RuntimeStorage {
			private:
			protected:
				std::vector<std::pair<dust::Table, int>> records;			// Member storage
				std::unordered_map<dust::Table, size_t> registry;			// Map of Table to reference

				// Gets the next "open" spot in the store
				size_t nxt_record(dust::Table);

				void markFree(size_t);
				bool validIndex(size_t);
				bool isCollectableResource(size_t);

			public:
				TableStorage();

				// Create a reference
				size_t loadRef(dust::Table t);

				// Assign/Reassign references
				size_t setRef(size_t ref, dust::Table t);

				// Dereference
				dust::Table deref(size_t ref);

				// Inherited from RuntimeStorage
				size_t size();
				int numRefs(size_t ref);
				void incRef(size_t ref);
				void decRef(size_t ref);
		};

	}
}