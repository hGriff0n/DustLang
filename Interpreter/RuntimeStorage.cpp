#include "RuntimeStorage.h"

namespace dust {
	namespace impl {

		RuntimeStorage::RuntimeStorage() {}

		size_t RuntimeStorage::pop() {
			auto it = --open.end();
			auto ret = *it;
			open.erase(it);
			return ret;
		}

		void RuntimeStorage::tryMarkFree(size_t idx) {
			if (isCollectableResource(idx)) markFree(idx);
		}

		int RuntimeStorage::numRecords() {
			return size() - collected();
		}

		int RuntimeStorage::collected() {
			return open.size();
		}
	}
}