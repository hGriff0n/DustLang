#include "DualGC.h"

#include <algorithm>
#include "Exceptions\runtime.h"

namespace dust {
	namespace impl {
			size_t GC::getIncr() {
				if (c_idx > curr.capacity()) c_idx = 0;
				return std::min(c_idx + std::min((size_t)32, curr.capacity() / 4), curr.capacity());
			}

			StorageBase& GC::getCurrentTarget() {
				if (collection_target == 0)
					return strings;
				if (collection_target == 1)
					return tables;
				return funcs;
			}

			GC::GC() : strings{}, curr{ strings } {}

			int GC::run(bool f) {
				curr = getCurrentTarget();

				auto ret = f ? incrParse() : stopWorld();


				return ret;
			}

			int GC::stopWorld() {
				size_t idx = 0, end = curr.capacity();
				int inital_collected = curr.numCollected();

				// static const int NO_RUN = 20;
				// if (start >= NO_RUN) return 0;

				while (idx != end)
					curr.try_markFree(idx++);

				return curr.numCollected() - inital_collected;
			}

			int GC::incrParse() {c_end = getIncr();
				int initial_collected = curr.numCollected();

				while (c_idx != c_end)
					curr.try_markFree(c_idx++);

				if (c_end == curr.capacity()) c_idx = 0;
				return curr.numCollected() - initial_collected;
			}

			TableStorage& GC::getTables() {
				return tables;
			}

			StringStorage& GC::getStrings() {
				return strings;
			}

			FuncStorage& GC::getFunctions() {
				return funcs;
			}

	}
}