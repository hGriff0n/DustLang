#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <set>
#include <functional>

auto pop(std::set<size_t, std::greater<size_t>>&);

// TODO/Considerations
	// Change open from an std::set to an std::vector
		// Then have GC::run call std::sort on the vector once the garbage collection has run
		// I could move the "#include <functional> into the .cpp file
		// The sorted vector may perform better than the set as the set must maintain order on insertion
	// Have RuntimeStorage implement the C++ Allocater interface
	// Determine the actual usefulness of temporaries as currently implemented
	// Consider decoupling GC and RuntimeStorage
		// I can possibly generalize the garbage collecter with an interface similar to _GC

namespace dust {
	namespace impl {
		struct str_record;

		class RuntimeStorage {
			private:
			protected:
				std::vector<str_record*> store;
				std::unordered_map<std::string, size_t> registry;
				std::set<size_t, std::greater<size_t>> open;

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
				size_t loadRef(std::string);							// New string (may return an old record)

				size_t setRef(size_t, size_t);							// Assign a ref
				size_t setRef(size_t, str_record*);						// Assign a temporary
				size_t setRef(size_t, std::string);						// Assign a string

				size_t combine(size_t, size_t);							// Add two refs (may reuse memory ???)
				size_t combine(size_t, std::string);					// Add a ref and a string
				size_t combine(std::string, std::string);				// Add two strings (???)
																		//size_t combine(size_t, str_record*);

				// TEMPORARIES
				str_record* tempRef(std::string);						// Generate a temporary
				str_record* tempRef(str_record*);						// Generate a temporary
				str_record* tempRef(size_t);							// Generate a temporary

				void setTemp(str_record*, str_record*);					// Set a temporary
				void setTemp(str_record*, size_t);						// Set a temporary
				void setTemp(str_record*, std::string);					// Set a temporary

				void addTemp(str_record*, size_t);						// Add a ref to a temporary
				void addTemp(str_record*, str_record*);					// Add two temporaries
				void addTemp(str_record*, std::string);					// Add a string to the temporary

				void delTemp(str_record*);								// Necessary because str_record's destructor is "hidden"

				// REFERENCE COUNTING
				void incRef(size_t);
				void decRef(size_t);

				// EXTRACTION
				std::string deref(str_record*);
				std::string deref(size_t);

				// Debug functions
				int num_records();
				int num_refs(std::string);
				int collected();
				void printAll();
		};

		//*/
		class GC : public RuntimeStorage {
			private:
				int c_idx = 0, c_end = 0;

			protected:
				size_t getIncr();

			public:
				GC();

				int run(bool = false);
				int stopWorld();
				int incrParse();
		};

		//*/


		class _GC {
			private:
			public:
				// _GC();
				// int run(RuntimeStorage&);
				// int run();
		};

	}
}