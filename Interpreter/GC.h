#pragma once

#include "Storage.h"

// TODO/Considerations
	// Have RuntimeStorage implement the C++ Allocater interface
	// Determine the actual usefulness of temporaries as currently implemented
	// Consider decoupling GC and RuntimeStorage
		// I can possibly generalize the garbage collecter with an interface similar to _GC

namespace dust {
	namespace impl {
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
			public:
				// _GC();
				// int run(RuntimeStorage&);
				// int run();
		};

	}
}