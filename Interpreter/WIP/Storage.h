#pragma once

#include <string>

//#define USE_GC_STACK

namespace dust {
	namespace impl {
		struct str_record;
		str_record* nxt_record();
		//str_record* init(str_record*, std::string, size_t);

		str_record* makeRecord(std::string);
		str_record* set(str_record*, str_record*);
		str_record* set(str_record*, std::string);
		str_record* combine(str_record*, str_record*);

		std::string deref(str_record*);

		void incRef(str_record*);
		void decRef(str_record*);
		str_record* delRef(str_record*);

		// DEBUG
		int num_records();
		int num_bins();
		void num_refs(std::string);
		void printAll();
		//void test();

	}
}