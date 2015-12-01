#include "DualGC.h"

#include <algorithm>
#include "Exceptions\runtime.h"

namespace dust {
	namespace impl {
			size_t GC::getIncr() {
				if (c_idx > curr.size()) c_idx = 0;
				return std::min(c_idx + std::min((size_t)32, curr.size() / 4), curr.size());
				//if (c_idx > curr.capacity()) c_idx = 0;
				//return std::min(c_idx + std::min((size_t)32, curr.capacity() / 4), curr.capacity());
			}

			GC::GC() : strings{}, curr{ strings } {}

			int GC::run(bool f) {
				curr = collect_strings ? strings : (RuntimeStorage&)tables;
				//curr = collect_strings ? strings : (StorageBase&)tables;

				auto ret = f ? incrParse() : stopWorld();


				return ret;
			}

			int GC::stopWorld() {
				size_t idx = 0, end = curr.size();
				int start = curr.collected();
				//size_t idx = 0, end = curr.capacity();
				//int inital_collected = curr.numCollected();

				// static const int NO_RUN = 20;
				// if (start >= NO_RUN) return 0;

				while (idx != end)
					curr.try_markFree(idx++);

				return curr.collected() - start;
				//return curr.numCollected() - inital_collected;
			}

			int GC::incrParse() {c_end = getIncr();
				int start = curr.collected();
				//int initial_collected = curr.numCollected();

				while (c_idx != c_end)
					curr.try_markFree(c_idx++);

				if (c_end == curr.size()) c_idx = 0;
				return curr.collected() - start;
				
				//if (c_end == curr.capacity()) c_idx = 0;
				//return curr.numCollected() - initial_collected;
			}

			TableStorage& GC::getTables() {
				return tables;
			}

			StringStorage& GC::getStrings() {
				return strings;
			}

	}
}