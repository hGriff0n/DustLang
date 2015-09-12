#include "GC.h"

#include <functional>
#include <algorithm>

namespace dust {
	namespace impl {
		size_t GC::getIncr() {
			return std::min(c_idx + std::min((size_t)32, lastIndex() / 4), lastIndex());
		}

		GC::GC() {}

		int GC::run(bool f) {
			static std::greater<size_t> comp;

			auto ret = f ? incrParse() : stopWorld();
			if (ret > 0) std::sort(open.begin(), open.end(), comp);
			return ret;
		}

		int GC::stopWorld() {
			size_t idx = 0, end = lastIndex();
			int start = open.size();

			// static const int NO_RUN = 20;
			// if (start >= NO_RUN) return 0;

			while (idx != end)
				try_mark_free(idx++);

			return open.size() - start;
		}

		int GC::incrParse() {
			c_end = getIncr();
			int start = open.size();

			while (c_idx != c_end)
				try_mark_free(c_idx++);

			if (c_end == lastIndex()) c_idx = 0;
			return open.size() - start;
		}

	}
}
