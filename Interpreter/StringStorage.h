#pragma once

#include "RuntimeStorage.h"
#include <string>

namespace dust {
	namespace impl {
		struct str_record;

		/*
		 * RuntimeStorage class specialized for storing std::strings (through str_record*)
		 */
		class StringStorage : public RuntimeStorage {
			private:
			protected:
				std::vector<str_record*> store;							// Storage
				std::unordered_map<std::string, size_t> registry;		// Map of string to reference id

				// Gets the next "open" spot in the store
				size_t nxtRecord(std::string);

				void markFree(size_t);
				bool validIndex(size_t);
				bool isCollectableResource(size_t);

			public:
				StringStorage();

				// Create a reference
				size_t loadRef(std::string s);

				// Assign/Reassign references
				size_t setRef(size_t ref, size_t s);							// Assign a ref
				size_t setRef(size_t ref, std::string s);						// Assign a string

				// Dereference
				std::string deref(size_t ref);

				// Overload of numRefs for a string
				int numRefs(std::string s);

				// Inherited from RuntimeStorage
				size_t size();
				int numRefs(size_t ref);
				void incRef(size_t ref);
				void decRef(size_t ref);
		};

	}
}
