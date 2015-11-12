#pragma once

#include "StringStorage.h"
#include "TableStorage.h"

namespace dust {
	namespace impl {

		class GC {
			private:
				StringStorage strings;
				TableStorage tables;
				RuntimeStorage* curr = nullptr;
				bool collect_strings = true;
				size_t c_idx = 0, c_end = 0;

				size_t getIncr();

			protected:
			public:
				GC();
				int run(bool f = false);
				int stopWorld();
				int incrParse();

				TableStorage& getTables();
				StringStorage& getStrings();
		};

	}
}