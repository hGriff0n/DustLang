#pragma once

#pragma once

#include <vector>

namespace dust {
	namespace impl {

		/*
		 * Base class for specifying a container of collectable resources
		 * Provides the interface for the gc algorithms
		 */
		class StorageBase {
			protected:
				// Stack for specifying open slots in memory
				std::vector<size_t> open;

				// Pop an element from the stack
				size_t pop();

				// Used in implementing try_markFree
				virtual void markFree(size_t) =0;
				virtual bool validIndex(size_t) =0;
				virtual bool isCollectableResource(size_t) =0;

			public:
				StorageBase();
				
				// Size of data members
				int size();
				int numCollected();
				virtual size_t capacity() = 0;

				// Attempt to free the given reference
				void try_markFree(size_t ref);

				// Adjust the number of references to the given references
				virtual void incRef(size_t ref) =0;
				virtual void decRef(size_t ref) =0;
				virtual int numRefs(size_t ref) =0;
		};

	}
}