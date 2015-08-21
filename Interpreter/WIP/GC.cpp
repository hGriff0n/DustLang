#include "GC.h"
#include <unordered_map>

#include <iostream>

namespace dust {
	namespace impl {
		struct str_record {
			int numRefs = 0;
			size_t bin;
			std::string s;
		};
		
		str_record* init(str_record* r, std::string s, size_t bin) {
			r->s = s;
			r->bin = bin;
			return r;
		}

		RuntimeStorage::RuntimeStorage() {
			s_store.emplace_back();
		}

		str_record* RuntimeStorage::nxt_record() {
			if (!open.empty())
				return pop(open);

			if (s_store[curr].full()) {
				s_store.emplace_back();
				++curr;
			}

			return s_store[curr].open++;
		}

		str_record* RuntimeStorage::delRef(str_record* r) {
			decRef(r);

			// If r was the only instance of the string
			if (r->numRefs == 0) {
				registry.erase(r->s);

				// Free the register for future allocations
				open.push(r);
			}

			// return r = nullptr;
			return nullptr;
		}

#ifdef USE_EXP_TEMPS
		static bin<str_record, 5> temp;

		str_record* RuntimeStorage::tempRef(std::string s) {
			if (temp.full()) throw std::string{ "Attempt to allocate too many temporaries" };
			setTemp(temp.open, s);
			return temp.open++;
		}

		void RuntimeStorage::setTemp(str_record* r, std::string s) {
			r->s = s;
		}

		void RuntimeStorage::flushTemporaries() {
			temp.open = temp.store;
		}
#endif

		int RuntimeStorage::num_records() {
			int sum = 0;

			for (auto b : s_store)
				sum += b.num_records();

			return sum;
		}

		int RuntimeStorage::num_bins() {
			return s_store.size();
		}

		void RuntimeStorage::printAll() {
			for (auto it : registry)
				std::cout << "bin: " << it.second->bin << ", refs: " << it.second->numRefs << " :: \"" << it.first << "\"\n";
		}

		int RuntimeStorage::collected() {
			return open.size();
		}

		str_record* RuntimeStorage::loadRef(std::string s) {
			return setRef(nullptr, s);
		}

		str_record* RuntimeStorage::setRef(str_record* r1, str_record* r2) {
			return setRef(r1, r2->s);
		}

		str_record* RuntimeStorage::setRef(str_record* r, std::string s) {
			if (r) decRef(r);

			// If the string already exists, get the record
			if (registry.count(s) > 0)
				r = registry[s];

			// If the record was the only reference, reuse the record
			else if (r && r->numRefs == 0) {
				registry.erase(r->s);
				init(registry[s] = r, s, r->bin);

			} else	// Otherwise get new record
				r = init(registry[s] = nxt_record(), s, curr);

			incRef(r);
			return r;
		}

		str_record* RuntimeStorage::combine(str_record* r1, str_record* r2) {
			if (r1->numRefs == 1) {
				registry.erase(r1->s);
				r1->s += r2->s;
				return registry[r1->s] = r1;

			} else
				return setRef(r1, r1->s + r2->s);
		}


		GC::GC() {}

		int GC::run() {
			/*
			for (auto& b : gcStart()) {
				str_record* t = b.store;

				while (t != b.end()) {
					if (t->numRefs == 0)
						delRef(t); / markFree(t);

					++t;
				}
			}
			
			*/

			return 0;
		}


		void incRef(str_record* r) {
			if (!r) throw std::string{ "Nullptr passed to incRef" };
			r->numRefs++;
		}

		void decRef(str_record* r) {
			if (!r) throw std::string{ "Nullptr passed to incRef" };
			r->numRefs--;
		}

		std::string deref(str_record* r) {
			if (!r) throw std::string{ "Nullptr passed to deref" };
			return r->s;
		}

	}
}