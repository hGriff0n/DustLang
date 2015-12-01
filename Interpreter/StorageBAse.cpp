#include "StorageBase.h"

namespace dust {
	namespace impl {

		StorageBase::StorageBase() {}

		size_t StorageBase::pop() {
			size_t back = open.back();
			open.pop_back();
			return back;
		}

		void StorageBase::try_markFree(size_t idx) {
			if (isCollectableResource(idx)) markFree(idx);
		}

		int StorageBase::size() {
			return capacity() - numCollected();
		}

		int StorageBase::numCollected() {
			return open.size();
		}
	}
}