#pragma once

#include "RuntimeStorage.h"
#include <string>

namespace dust {
	namespace impl {
		struct str_record;

		/*
		template <typename s, typename k = s>
		class RuntimeStorage {
			private:
			protected:
				std::vector<s> store;
				std::unordered_map<k, size_t> registry;
				std::vector<size_t> open;
				...
		}
		*/

		/*
		 * Stores and controls access to str_record (and other allocable resources)
		 */
		class StringStorage : public RuntimeStorage {
			private:
			protected:
				std::vector<str_record*> store;
				std::unordered_map<std::string, size_t> registry;

				bool isCollectableResource(size_t);

				void markFree(size_t);
				bool validIndex(size_t);
				size_t nxt_record(std::string);

			public:
				StringStorage();

				// INITIALIZATION/MODIFICATION
				//size_t loadRef(size_t);											// New reference
				size_t loadRef(std::string s);

				size_t setRef(size_t ref, size_t s);							// Assign a ref
				size_t setRef(size_t ref, std::string s);						// Assign a string

				// REFERENCE COUNTING
				void incRef(size_t ref);
				void decRef(size_t ref);

				// EXTRACTION
				std::string deref(size_t ref);

				// Debug functions
				size_t size();
				int num_refs(size_t ref);
				int num_refs(std::string s);
		};

	}
}
