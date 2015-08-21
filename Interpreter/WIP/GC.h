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

		// This is std::array
		template <typename T, size_t MAX_NUM>
		struct bin {
			T* store, *open;

			bin() {
				store = new T[MAX_NUM];
				open = store;
			}

			int num_records() { return open - store; }
			bool full() { return num_records() == MAX_NUM; }
			T* end() { return store + MAX_NUM; }
		};

		// class RuntimeEnviron ???

		// template <typename T>
		class RuntimeStorage {
			private:
				static const int BIN_SIZE = 128;

				std::vector<bin<str_record, BIN_SIZE>> s_store;
				std::unordered_map<std::string, str_record*> registry;				// Technically, the registry is the only structure I really need
				std::stack<str_record*> open;
				size_t curr = 0;

			protected:
			public:
				RuntimeStorage();
				
				// STRING RECORDS
				// CREATION AND DELETION
				str_record* nxt_record();
				str_record* delRef(str_record*);				// Current Method of implementing temporary records

				// INITIALIZATION
				str_record* loadRef(std::string);
				str_record* setRef(str_record*, str_record*);
				str_record* setRef(str_record*, std::string);
				str_record* combine(str_record*, str_record*);

				// TEMPORARIES
#ifdef USE_EXP_TEMPS
				str_record* tempRef(std::string);
				void setTemp(str_record*, std::string);
				void flushTemporaries();
#endif

				// FOR USE BY THE GARBAGE COLLECTOR
				// Some method of getting the store to work on
				// void mark_free(str_record*);

				// Debug functions
				int num_records();
				int num_bins();
				int collected();
				//void num_refs(std::string);
				void printAll();
		};
		
		void incRef(str_record*);
		void decRef(str_record*);
		std::string deref(str_record*);


		class GC : public RuntimeStorage {
			private:
			public:
				GC();

				int run();
		};

	}

}