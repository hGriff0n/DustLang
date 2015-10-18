#pragma once

#include <vector>
#include <unordered_map>
#include <string>

auto pop(std::vector<size_t>&);

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
		class RuntimeStorage {
			private:
			protected:
				std::vector<str_record*> store;
				std::unordered_map<std::string, size_t> registry;
				std::vector<size_t> open;

				size_t lastIndex();
				void mark_free(size_t);
				void try_mark_free(size_t);

				bool isCollectableResource(size_t);
				bool validIndex(size_t);

				size_t nxt_record(std::string);

			public:
				RuntimeStorage();

				// INITIALIZATION/MODIFICATION
				//size_t loadRef(size_t);								// New reference
				size_t loadRef(std::string s);
				//size_t loadRef(str_record*);

				size_t setRef(size_t idx, size_t s);							// Assign a ref
				size_t setRef(size_t idx, str_record* s);						// Assign a temporary
				size_t setRef(size_t idx, std::string s);						// Assign a string

				// REFERENCE COUNTING
				void incRef(size_t r);
				void decRef(size_t r);

				// EXTRACTION
				std::string deref(size_t idx);
				std::string deref(str_record* s);

				// Debug functions
				int num_records();
				int num_refs(std::string s);
				int collected();
				void printAll();
		};

	}
}
