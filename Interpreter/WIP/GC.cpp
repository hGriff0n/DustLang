#include "GC.h"

#include <array>
#include <iostream>

namespace dust {
	namespace impl {
		struct str_record {
			int numRefs = 0;
			std::string s;
		};

		str_record* init(str_record* r, std::string s) {
			r->s = s;
			return r;
		}

		RuntimeStorage::RuntimeStorage() {}

		str_record* RuntimeStorage::nxt_record() {
			if (!open.empty()) return pop(open);

			store.emplace_back(new str_record{});
			return store.back();
		}

		// Might remove depending on how I implement garbage collection.
		// If I move to a delete-allocate model, I'd have to store the index where the record is stored for this to be worthwhile
		// With the current model, this method works fine. The real consideration is the efficacy of explicit temporaries when evaluating ASTs.
		// If the explicit temporary system is usable, then I believe I should move to the delete-allocate model (and remove this method).
		str_record* RuntimeStorage::delRef(str_record* r) {
			decRef(r);

			if (r->numRefs == 0) {
				registry.erase(r->s);

				mark_free(r);

				// Uncomment if I change open to instead store the index where r existed
				// delete r;
			}

			return r = nullptr;
		}

		str_record* RuntimeStorage::loadRef(std::string s) {
			return setRef(nullptr, s);
		}

		str_record* RuntimeStorage::setRef(str_record* r1, str_record* r2) {
			return setRef(r1, r2->s);
		}

		str_record* RuntimeStorage::setRef(str_record* r, std::string s) {
			if (r) decRef(r);

			// It is the garbage collector's job to mark records as "free" not this function
			// The only action this function should take is reusing the record if possible

			// If the string already exists, get the record
			if (registry.count(s) > 0)
				r = registry[s];

			// If the record was the only reference, reuse it
			else if (r && r->numRefs == 0) {
				registry.erase(r->s);
				init(registry[s] = r, s);

			} else	// Otherwise get a new record
				r = init(registry[s] = nxt_record(), s);

			// Increment and return the new record
			incRef(r);
			return r;
		}

		str_record* RuntimeStorage::combine(str_record* r1, str_record* r2) {
			// Modify the string in-place if r1 is the last reference
			// what if registry.count(r1->s + r2->s) > 0 ???
			if (r1->numRefs == 1) {
				registry.erase(r1->s);
				r1->s += r2->s;
				return registry[r1->s] = r1;

			} else
				return setRef(r1, r1->s + r2->s);
		}

#ifdef USE_EXP_TEMPS
		static std::array<str_record*, 5> temp;
		static int nxt_tr = -1;

		void RuntimeStorage::setTemp(str_record* r, std::string s) {
			r->s = s;
		}

		void RuntimeStorage::delTemps() {
			while (nxt_tr >= 0) delete temp[nxt_tr--];
		}

		str_record* RuntimeStorage::tempRef(std::string s) {
			if (++nxt_tr == temp.size()) throw std::string{"Attempt to allocate too many temporaries"};

			return init(temp[nxt_tr] = new str_record, s);
		}

#endif

		int RuntimeStorage::num_records() {
			return store.size() - open.size();
		}

		int RuntimeStorage::num_refs(std::string s) {
			return registry.count(s) ? registry[s]->numRefs : -1;
		}

		void RuntimeStorage::printAll() {
			for (auto it : registry)
				std::cout << "refs: " << it.second->numRefs << " :: \"" << it.first << "\"\n";
		}

		int RuntimeStorage::collected() {
			return open.size();
		}

		int RuntimeStorage::lastIndex() {
			return store.size();
		}

		void RuntimeStorage::mark_free(str_record* r) {
			if (!r) throw std::string{ "Nullptr exception" };
			open.push(r);
		}

		void RuntimeStorage::try_mark_free(str_record* r) {
			if (r && r->numRefs == 0) mark_free(r);
		}


		void incRef(str_record* r) {
			if (!r) throw std::string{ "Nullptr exception" };
			r->numRefs++;
		}
		void decRef(str_record* r) {
			if (!r) throw std::string{ "Nullptr exception" };
			r->numRefs--;
		}
		std::string deref(str_record* r) {
			if (!r) throw std::string{ "Nullptr exception" };
			return r->s;
		}


		GC::GC() {}

		int GC::run() {
			return stopWorld();
			//return incrParse();
		}
		
		int GC::stopWorld() {
			int idx = 0, end = lastIndex(), total = 0;

			//static const int NO_RUN = 20;
			//if (end - NO_RUN <= idx) return 0;

			auto b = store.begin();

			while (idx != end) {
				if ((*b)->numRefs <= 0) {
					registry.erase((*b)->s);
					mark_free(*b);				// delete (*b); mark_free(idx);
					++total;
				}

				++b; ++idx;
			}

			return total;
		}

		int GC::incrParse() {
			return 0;
		}

	}
}