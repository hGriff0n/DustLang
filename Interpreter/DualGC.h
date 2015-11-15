#pragma once

#include "StringStorage.h"
#include "TableStorage.h"

namespace dust {
	namespace impl {

		class GC {
			private:
				StringStorage strings;
				TableStorage tables;
				RuntimeStorage* curr = nullptr;			// Storage used by the collection algorithms. Can rotate between strings and tables based on value of collect_strings

				bool collect_strings = true;			// Switch to determine which storage to collect (no way to change it in the current interface)
				size_t c_idx = 0, c_end = 0;

				// Get the block size for incrParse
				size_t getIncr();

			protected:
			public:
				GC();

				// Run the garbage collector. Defaults to running stopWorld
				int run(bool f = false);

				// Run a stopworld collection
				int stopWorld();
				// Run an incremental collection
				int incrParse();

				// Get the internal storage members
				TableStorage& getTables();
				StringStorage& getStrings();
		};

	}
}