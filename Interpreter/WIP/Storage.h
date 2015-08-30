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
				//using temporary = str_record*;

				// INITIALIZATION/MODIFICATION
				//size_t loadRef(size_t);								// New reference
				size_t loadRef(std::string);
				//size_t loadRef(str_record*);

				size_t setRef(size_t, size_t);							// Assign a ref
				size_t setRef(size_t, str_record*);						// Assign a temporary
				size_t setRef(size_t, std::string);						// Assign a string

				// TEMPORARIES
				str_record* tempRef(size_t);							// Generate a temporary
				str_record* tempRef(std::string);
				str_record* tempRef(str_record*);

				void setTemp(str_record*, size_t);						// Set a temporary
				void setTemp(str_record*, std::string);
				void setTemp(str_record*, str_record*);

				void appTemp(str_record*, size_t);						// Append to a temporary
				void appTemp(str_record*, std::string);
				void appTemp(str_record*, str_record*);

				void delTemp(str_record*);								// Necessary because str_record's destructor is "hidden"

				// REFERENCE COUNTING
				void incRef(size_t);
				void decRef(size_t);

				// EXTRACTION
				std::string deref(size_t);
				std::string deref(str_record*);

				// Debug functions
				int num_records();
				int num_refs(std::string);
				int collected();
				void printAll();
		};

	}
}