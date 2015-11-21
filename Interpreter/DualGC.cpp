#include "DualGC.h"

#include <algorithm>

namespace dust {
	namespace impl {
			size_t GC::getIncr() {
				if (c_idx > curr->size()) c_idx = 0;
				return std::min(c_idx + std::min((size_t)32, curr->size() / 4), curr->size());
			}

			GC::GC() {}

			int GC::run(bool f) {
				curr = collect_strings ? (RuntimeStorage*)&strings : (RuntimeStorage*)&tables;

				auto ret = f ? incrParse() : stopWorld();


				return ret;
			}

			int GC::stopWorld() {
				if (!curr) throw;

				size_t idx = 0, end = curr->size();
				int start = curr->collected();

				// static const int NO_RUN = 20;
				// if (start >= NO_RUN) return 0;

				while (idx != end)
					curr->try_markFree(idx++);

				return curr->collected() -start;
			}

			int GC::incrParse() {
				if (!curr) throw;

				c_end = getIncr();
				int start = curr->collected();

				while (c_idx != c_end)
					curr->try_markFree(c_idx++);

				if (c_end == curr->size()) c_idx = 0;
				return curr->collected() - start;
			}

			TableStorage& GC::getTables() {
				return tables;
			}

			StringStorage& GC::getStrings() {
				return strings;
			}

	}
}