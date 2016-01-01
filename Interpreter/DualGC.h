#pragma once

#include "Collector.h"

namespace dust {
	namespace impl {

		class GC {
			private:
				StringStorage strings;
				TableStorage tables;
				FuncStorage funcs;
				StorageBase& curr;						// Storage used by the collection algorithms. Can rotate between strings and tables based on value of collect_strings

				short collection_target = 0;			// Switch to determine which storage to collect (0 = string, 1 = table, 2 = funcs)
				size_t c_idx = 0, c_end = 0;

				// Get the block size for incrParse
				size_t getIncr();

				// Abstract multiple storage options
				StorageBase& getCurrentTarget();

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
				FuncStorage& getFunctions();
		};

	}
}