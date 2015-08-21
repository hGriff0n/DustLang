#include "_GC.h"

#include <array>
#include <iostream>

namespace dust {
	namespace test {
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
			if (!open.empty())
				return pop(open);

			store.emplace_back(new str_record{});
			return store.back();
		}

		str_record* RuntimeStorage::delRef(str_record* r) {
			decRef(r);

			if (r->numRefs == 0) {
				registry.erase(r->s);

				open.push(r);

				// This would cause memory errors as the memory gets reused, not reallocated
					// If I change open to refer to the open indices (not the *), then this would work (and would actually be necessary)
					// But I'm not able to access where r is stored (here at least)
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

			// If the record was the only reference, reuse the record
			else if (r && r->numRefs == 0) {
				registry.erase(r->s);
				init(registry[s] = r, s);

			} else	// Otherwise get new record
				r = init(registry[s] = nxt_record(), s);

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

#ifdef USE_EXP_TEMPS
		static std::array<str_record*, 5> temp;
		static int nxt_tr = 0;

		str_record* RuntimeStorage::tempRef(std::string s) {
			if (nxt_tr == temp.size()) throw std::string{"Attempt to allocate too many temporaries" };

			temp[nxt_tr] = new str_record{};

			return init(temp[nxt_tr++], s);
		}

		void RuntimeStorage::setTemp(str_record* r, std::string s) {
			r->s = s;
		}

		void RuntimeStorage::flushTemporaries() {
			while (nxt_tr >= 0)
				delete temp[nxt_tr--];

			++nxt_tr;
		}
#endif

		int RuntimeStorage::num_records() {
			return store.size() - open.size();
		}

		int RuntimeStorage::num_refs(std::string s) {
			return registry[s]->numRefs;
		}

		void RuntimeStorage::printAll() {
			for (auto it : registry)
				std::cout << "refs: " << it.second->numRefs << " :: \"" << it.first << "\"\n";
		}

		int RuntimeStorage::collected() {
			return open.size();
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
			return 0;
		}

	}
}