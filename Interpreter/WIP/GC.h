#pragma once

#include <string>
#define USE_EXP_TEMPS

#include <vector>
#include <unordered_map>
#include <stack>
#include <set>

#define INT_STACK

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
				
				std::stack<size_t> open;
				//std::set<size_t, std::greater<size_t>> open;

				// FOR USE BY THE GARBAGE COLLECTOR (Not needed if open stays as protected)
				size_t lastIndex();

				void mark_free(size_t);
				void try_mark_free(size_t);
				bool isCollectableRecord(size_t);

			public:
			str_record* test(size_t idx) { return store[idx]; }
				RuntimeStorage();

				// STRING RECORDS
				// CREATION AND DELETION
				str_record* nxt_record();

				// INITIALIZATION
				str_record* loadRef(std::string);
				str_record* setRef(str_record*, str_record*);
				str_record* setRef(str_record*, std::string);
				str_record* combine(str_record*, str_record*);	// Only pass temporaries as the second argument

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
		bool isDelRef(str_record*);


		class GC : public RuntimeStorage {
			private:
				size_t c_idx = 0, c_end = 0;

			protected:
				size_t getIncr();

			public:
				GC();

				int run(bool = false);
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