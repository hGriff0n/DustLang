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
	namespace test {
		struct str_record;

		// class RuntimeEnviron ???

		// template <typename T>
		class RuntimeStorage {
			private:
				std::vector<str_record*> store;
				std::unordered_map<std::string, str_record*> registry;
				std::stack<str_record*> open;

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
				// Or perhaps the registry would be more useful???
				// void mark_free(str_record*);

				// Debug functions
				int num_records();
				int num_refs(std::string);
				int collected();
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