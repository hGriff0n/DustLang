#include "TableStorage.h"

#include "Exceptions\runtime.h"

namespace dust {
	namespace impl {

		TableStorage::TableStorage() {}

		bool TableStorage::validIndex(size_t idx) {
			return idx < records.size();
		}

		size_t expand(std::vector<std::pair<dust::Table, int>>& vec) {
			vec.push_back(std::make_pair(nullptr, 0));
			return vec.size() - 1;
		}

		size_t TableStorage::nxt_record(dust::Table t) {
			size_t alloc = open.empty() ? expand(records) : pop();
			records[alloc] = std::make_pair(t, 0);
			return alloc;
		}

		size_t TableStorage::loadRef(dust::Table t) {
			if (!t) throw error::null_exception{ "Attempt to load a reference to a nullptr" };
			size_t ref = (registry.count(t) > 0) ? registry[t] : (registry[t] = nxt_record(t));
			incRef(ref);
			return ref;
		}

		// Implement similar to StringStorage::setRef
		size_t TableStorage::setRef(size_t ref, dust::Table t) {
			return size_t();
		}

		dust::Table TableStorage::deref(size_t idx) {
			if (!validIndex(idx)) throw error::storage_access_error{ "Invalid Record Access" };
			return records[idx].first;
		}

		void TableStorage::incRef(size_t idx) {
			if (!validIndex(idx)) throw error::storage_access_error{ "Invalid Record Access" };
			++records[idx].second;
		}

		void TableStorage::decRef(size_t idx) {
			if (!validIndex(idx)) throw error::storage_access_error{ "Invalid Record Access" };
			--records[idx].second;
		}

		size_t TableStorage::size() {
			return records.size();
		}

		int TableStorage::numRefs(size_t loc) {
			if (!validIndex(loc)) return 0;
			return records[loc].second;
		}

		void TableStorage::markFree(size_t idx) {
			registry.erase(records[idx].first);
			delete records[idx].first;
			records[idx].first = nullptr;
			open.push_back(idx);
		}

		bool TableStorage::isCollectableResource(size_t idx) {
			return validIndex(idx) && records[idx].second < 1;
		}

	}
}