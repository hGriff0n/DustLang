#pragma once

#include <string>
#define USE_EXP_TEMPS

#include <vector>
#include <unordered_map>
#include <stack>

template <typename T>
T pop(std::stack<T>& s) {
	auto ret = s.top();
	s.pop();
	return ret;
}

namespace dust {
	namespace impl {
		struct str_record;

		// class RuntimeEnviron ???
		// class Allocater								// I can even implement the C++ Allocator interface (Might simplify the code, I would likely seperate this from the temporaries)

		// template <typename T>
		class RuntimeStorage {
			private:
			protected:
				std::vector<str_record*> store;					// can modify to std::array<str_record*, X> if needed
				std::unordered_map<std::string, str_record*> registry;

				// Might want to change to instead hold the open indices of store
					// Then the gc can delete the reference when it finds them instead of hoping for reuse
					// Might have to modify str_record* to store it's location in store (if I don't want to use explicit temporaries, ie. delRef)
				std::stack<str_record*> open;


				// FOR USE BY THE GARBAGE COLLECTOR (Not needed if open stays as protected)
				int lastIndex();
				void mark_free(str_record*);
				void try_mark_free(str_record*);

			public:
				RuntimeStorage();

				// STRING RECORDS
				// CREATION AND DELETION
				str_record* nxt_record();
				str_record* delRef(str_record*);				// Current Method of implementing temporary records. Some problems with Garbage Collection

				// INITIALIZATION
				str_record* loadRef(std::string);
				str_record* setRef(str_record*, str_record*);
				str_record* setRef(str_record*, std::string);
				str_record* combine(str_record*, str_record*);

				// EXPLICIT TEMPORARIES
#ifdef USE_EXP_TEMPS
				str_record* tempRef(std::string);
				void setTemp(str_record*, std::string);
				void delTemps();
#endif

				// Debug functions
				int num_records();
				int num_refs(std::string);
				int collected();
				void printAll();
		};

		
		// Should I move these into RuntimeStorage
		void incRef(str_record*);
		void decRef(str_record*);
		std::string deref(str_record*);


		class GC : public RuntimeStorage {
			private:
			public:
				GC();

				int run();
				int stopWorld();
				int incrParse();
		};


		class _GC {
			private:
				RuntimeStorage& storage;

			public:
				// _GC();
				// _GC& target(RuntimeStorage&);
				// int run();
		};

	}

}