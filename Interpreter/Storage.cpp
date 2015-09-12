#include "Storage.h"

#include "Exceptions\runtime.h"
#include <iostream> // for "printAll" only

auto pop(std::vector<size_t>& s) {
	auto it = --s.end();
	auto ret = *it;
	s.erase(it);
	return ret;
}

namespace dust {
	namespace impl {

		struct str_record {
			int numRefs = 0;
			std::string s;

			str_record() {}
			str_record(std::string str) : s{ str } {}
		};

		size_t RuntimeStorage::lastIndex() {
			return store.size();
		}

		void RuntimeStorage::mark_free(size_t idx) {
			registry.erase(store[idx]->s);
			delete store[idx];
			store[idx] = nullptr;
			open.push_back(idx);
		}

		void RuntimeStorage::try_mark_free(size_t idx) {
			if (isCollectableResource(idx)) mark_free(idx);
		}

		bool RuntimeStorage::isCollectableResource(size_t idx) {
			return validIndex(idx) && store[idx]->numRefs < 1;			// validIndex ensures that store[idx] != nullptr
		}

		bool RuntimeStorage::validIndex(size_t idx) {
			return idx < store.size() && store[idx];
		}

		size_t expand(std::vector<str_record*>& vec) {
			vec.push_back(nullptr);							// make an empty record and return the index
			return vec.size() - 1;
		}

		size_t RuntimeStorage::nxt_record(std::string s) {
			size_t alloc = open.empty() ? expand(store) : pop(open);
			store[alloc] = new str_record{ s };
			return alloc;
		}

		RuntimeStorage::RuntimeStorage() {}

		size_t RuntimeStorage::loadRef(std::string s) {
			size_t ref = (registry.count(s) > 0) ? registry[s] : (registry[s] = nxt_record(s));
			incRef(ref);
			return ref;
		}

		size_t RuntimeStorage::setRef(size_t idx, size_t s) {
			if (!validIndex(s)) throw error::storage_access_error{ "Invalid Record Access" };		// idx will be checked in setRef(size_t, std::string)
			return setRef(idx, store[s]->s);
		}

		size_t RuntimeStorage::setRef(size_t idx, str_record* s) {
			if (!s) throw error::null_exception{ "Nullptr Exception" };
			return setRef(idx, s->s);
		}

		size_t RuntimeStorage::setRef(size_t idx, std::string s) {
			decRef(idx);

			// If the string already exists in memory, get the reference
			if (registry.count(s) > 0)
				idx = registry[s];

			// If the old string was the only reference, reuse the index
			else if (store[idx]->numRefs == 0) {
				registry.erase(store[idx]->s);
				store[registry[s] = idx]->s = s;

			// Otherwise, get the next record
			} else
				registry[s] = idx = nxt_record(s);

			store[idx]->numRefs++;							// incRef. incRef involves a validIndex check that is unnecessary
			return idx;
		}

		str_record* RuntimeStorage::tempRef(std::string s) {
			auto ret = new str_record{};
			setTemp(ret, s);
			return ret;
		}

		str_record* RuntimeStorage::tempRef(str_record* s) {
			auto ret = new str_record{};
			setTemp(ret, s);
			return ret;
		}

		str_record* RuntimeStorage::tempRef(size_t s) {
			auto ret = new str_record{};
			setTemp(ret, s);
			return ret;
		}

		void RuntimeStorage::setTemp(str_record* t, str_record* s) {
			return setTemp(t, s->s);
		}

		void RuntimeStorage::setTemp(str_record* t, size_t s) {
			if (!validIndex(s)) throw std::string{ "Invalid Record Access" };
			return setTemp(t, store[s]->s);
		}

		void RuntimeStorage::setTemp(str_record* t, std::string s) {
			if (!t) throw std::string{ "Nullptr Exception" };
			t->s = s;
		}

		void RuntimeStorage::appTemp(str_record* t, size_t s) {
			if (!validIndex(s)) throw std::string{ "Invalid Record Access" };
			appTemp(t, store[s]->s);
		}

		void RuntimeStorage::appTemp(str_record* t, str_record* s) {
			if (!s) throw std::string{ "Nullptr Exception" };
			appTemp(t, s->s);
		}

		void RuntimeStorage::appTemp(str_record* t, std::string s) {
			if (!t) throw std::string{ "Nullptr Exception" };
			t->s += s;
		}

		void RuntimeStorage::delTemp(str_record* t) {
			delete t;
		}

		void RuntimeStorage::incRef(size_t r) {
			if (!validIndex(r)) throw std::string{ "Invalid Record Access" };
			store[r]->numRefs++;
		}

		void RuntimeStorage::decRef(size_t r) {
			if (!validIndex(r)) throw std::string{ "Invalid Record Access" };
			store[r]->numRefs--;
		}

		std::string RuntimeStorage::deref(str_record* s) {
			if (!s) throw std::string{ "Nullptr Exception" };
			return s->s;
		}

		std::string RuntimeStorage::deref(size_t idx) {
			if (!validIndex(idx)) throw std::string{ "Invalid Record Access" };
			return store[idx]->s;
		}

		int RuntimeStorage::num_records() {
			return store.size() - collected();
		}

		int RuntimeStorage::num_refs(std::string s) {
			return registry.count(s) > 0 ? store[registry[s]]->numRefs : -1;
		}

		int RuntimeStorage::collected() {
			return open.size();
		}

		void RuntimeStorage::printAll() {
			for (auto it : registry)
				std::cout << "refs: " << store[it.second]->numRefs << " :: \"" << it.first << "\"\n";
		}
	}
}
