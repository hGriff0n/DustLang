#include "StringStorage.h"

#include "Exceptions\runtime.h"


namespace dust {
	namespace impl {

		struct str_record {
			int num_refs = 0;
			std::string s;

			str_record() {}
			str_record(std::string str) : s{ str } {}
		};

		size_t StringStorage::size() {
			return store.size();
		}

		void StringStorage::markFree(size_t idx) {
			registry.erase(store[idx]->s);
			delete store[idx];
			store[idx] = nullptr;
			open.push_back(idx);
		}

		bool StringStorage::isCollectableResource(size_t idx) {
			return validIndex(idx) && store[idx]->num_refs < 1;			// validIndex ensures that store[idx] != nullptr
		}

		bool StringStorage::validIndex(size_t idx) {
			return idx < store.size() && store[idx];
		}

		size_t expand(std::vector<str_record*>& vec) {
			vec.push_back(nullptr);							// make an empty record and return the index
			return vec.size() - 1;
		}

		size_t StringStorage::nxtRecord(std::string s) {
			size_t alloc = open.empty() ? expand(store) : pop();
			store[alloc] = new str_record{ s };
			return alloc;
		}

		StringStorage::StringStorage() {}

		size_t StringStorage::loadRef(std::string s) {
			return (registry.count(s) > 0) ? registry[s] : (registry[s] = nxtRecord(s));
		}

		size_t StringStorage::setRef(size_t idx, size_t s) {
			if (!validIndex(s)) throw error::storage_access_error{ "Invalid Record Access" };		// idx will be checked in setRef(size_t, std::string)
			return setRef(idx, store[s]->s);
		}

		size_t StringStorage::setRef(size_t idx, std::string s) {
			decRef(idx);

			// If the string already exists in memory, get the reference
			if (registry.count(s) > 0)
				idx = registry[s];

			// If the old string was the only reference, reuse the index
			else if (store[idx]->num_refs == 0) {
				registry.erase(store[idx]->s);
				store[registry[s] = idx]->s = s;

			// Otherwise, get the next record
			} else
				registry[s] = idx = nxtRecord(s);

			store[idx]->num_refs++;							// incRef. incRef involves a validIndex check that is unnecessary
			return idx;
		}

		void StringStorage::incRef(size_t r) {
			if (!validIndex(r)) throw error::storage_access_error{ "Invalid Record Access" };
			store[r]->num_refs++;
		}

		void StringStorage::decRef(size_t r) {
			if (!validIndex(r)) throw error::storage_access_error{ "Invalid Record Access" };
			store[r]->num_refs--;
		}

		std::string StringStorage::deref(size_t idx) {
			if (!validIndex(idx)) throw error::storage_access_error{ "Invalid Record Access" };
			return store[idx]->s;
		}

		int StringStorage::numRefs(std::string s) {
			return numRefs(registry.count(s) > 0 ? registry[s] : -1);
		}

		int StringStorage::numRefs(size_t s) {
			return validIndex(s) ? store[s]->num_refs : 0;
		}

	}
}
