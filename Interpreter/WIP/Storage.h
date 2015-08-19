#pragma once

#include <string>

#define USE_BIN_STORAGE

namespace dust {
	namespace impl {
		struct str_record;

		str_record* makeRecord(std::string);
		str_record* set(str_record*, str_record*);
		str_record* set(str_record*, std::string);

		str_record* nxt_record();
		std::string deref(str_record*);
		void addRef(str_record*);
		void delRef(str_record*);

		// DEBUG
		int num_records();
		int num_bins();
		void num_refs(std::string);
		void printAll();

	}
}