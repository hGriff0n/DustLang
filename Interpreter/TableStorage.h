#pragma once

#include "RuntimeStorage.h"
#include "Table.h"

namespace dust {
	namespace impl {

		class TableStorage : public RuntimeStorage {
			private:
			protected:
				std::vector<std::pair<dust::Table, int>> records;
				std::unordered_map<dust::Table, size_t> registry;

				size_t nxt_record(dust::Table);

				void markFree(size_t);
				bool validIndex(size_t);
				bool isCollectableResource(size_t);

			public:
				TableStorage();

				size_t loadRef(dust::Table t);
				dust::Table deref(size_t ref);

				void incRef(size_t ref);
				void decRef(size_t ref);

				size_t size();
				int num_refs(size_t ref);
		};

	}
}