#include "GC.h"

#include <array>
#include <algorithm>
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
			if (!open.empty())
				return store[pop(open)] = new str_record{};
			
			store.push_back(new str_record{});
			return store.back();
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
			if (isDelRef(r1) || isDelRef(r2)) throw std::string{ "Deleted Record Exception" };

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

		size_t RuntimeStorage::lastIndex() {
			return store.size();
		}

		void RuntimeStorage::mark_free(size_t idx) {
			// if (!store[idx]) throw std::string{"Deleted Record exception"};
			registry.erase(store[idx]->s);
			delete store[idx];
			store[idx] = nullptr;
			open.push(idx);
		}

		void RuntimeStorage::try_mark_free(size_t idx) {
			if (isCollectableRecord(idx)) mark_free(idx);
		}

		bool RuntimeStorage::isCollectableRecord(size_t idx) {
			return store[idx] && store[idx]->numRefs <= 0;
		}


		void incRef(str_record* r) {
			if (isDelRef(r)) throw std::string{ "Deleted Record exception" };
			r->numRefs++;
		}
		void decRef(str_record* r) {
			if (isDelRef(r)) throw std::string{ "Deleted Record exception" };
			r->numRefs--;
		}
		std::string deref(str_record* r) {
			if (isDelRef(r)) throw std::string{ "Deleted Record exception" };			// I cannot rely on r == nullptr (especially as the gc can't assign every reference)
			return r->s;
		}
		bool isDelRef(str_record* r) {
			return !r || r->numRefs == -572662307 || r->numRefs < 0;
		}


		GC::GC() {}

		int GC::run(bool f) {
			return f ? incrParse() : stopWorld();
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

		size_t GC::getIncr() {
			return std::min(c_idx + std::min((size_t)32, lastIndex() / 4), lastIndex());
		}

		int GC::incrParse() {
			if (c_end == lastIndex()) c_idx = 0;
			c_end = getIncr();

			int start = open.size();

			while (c_idx != c_end)
				try_mark_free(c_idx++);

			return open.size() - start;
		}

	}
}