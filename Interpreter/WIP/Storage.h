#pragma once

#include <string>

//#define USE_GC_STACK
//#define USE_EXP_TEMPS

namespace dust {
	namespace impl {
		struct str_record;
		str_record* nxt_record();
		//str_record* init(str_record*, std::string, size_t);

		str_record* loadRef(std::string);
		str_record* setRef(str_record*, str_record*);
		str_record* setRef(str_record*, std::string);
		str_record* combine(str_record*, str_record*);

		std::string deref(str_record*);

		void incRef(str_record*);
		void decRef(str_record*);

		// TEMPORARIES
#ifdef USE_EXP_TEMPS
		str_record* tempRef(std::string);
		void setTemp(str_record*, std::string);
		void flushTemporaries();
#endif

		// ALT TEMPORARIES
		str_record* delRef(str_record*);

		// DEBUG
		int num_records();
		int num_bins();
		void num_refs(std::string);
		void printAll();
		//void test();

	}
}