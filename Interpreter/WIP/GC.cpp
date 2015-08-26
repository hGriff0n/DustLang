#include "GC.h"

#include <algorithm>
#include <iostream>

auto pop(std::set<size_t, std::greater<size_t>>& s) {
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
			// if (!store[idx]) throw std::string{"Deleted Record exception"};
			registry.erase(store[idx]->s);
			delete store[idx];
			store[idx] = nullptr;
			open.insert(idx);
		}

		void RuntimeStorage::try_mark_free(size_t idx) {
			if (isCollectableResource(idx)) mark_free(idx);
		}

		bool RuntimeStorage::isCollectableResource(size_t idx) {
			return validIndex(idx) && store[idx]->numRefs <= 0;
		}

		bool RuntimeStorage::validIndex(size_t idx) {
			return idx < store.size() && store[idx];
		}

		size_t expand(std::vector<str_record*>& vec) {
			vec.push_back(nullptr);
			return vec.size() - 1;
		}

		size_t RuntimeStorage::nxt_record(std::string s) {
			size_t alloc = open.empty() ? expand(store) : pop(open);

			store[alloc] = new str_record{ s };

			return alloc;
		}

		RuntimeStorage::RuntimeStorage() {}

		size_t RuntimeStorage::loadRef(std::string s) {
			size_t ret = registry.count(s) > 0 ? registry[s] : registry[s] = nxt_record(s);
			incRef(ret);
			return ret;
		}

		size_t RuntimeStorage::setRef(size_t idx, size_t s) {
			if (!validIndex(idx)) throw std::string{ "Invalid Record Access" };
			return setRef(idx, store[s]->s);
		}

		size_t RuntimeStorage::setRef(size_t idx, std::string s) {
			if (!validIndex(idx)) throw std::string{ "Invalid Record Access" };
			decRef(idx);

			if (registry.count(s) > 0)
				idx = registry[s];

			else if (store[idx]->numRefs == 0) {
				registry.erase(store[idx]->s);
				store[registry[s] = idx]->s = s;

			} else
				registry[s] = idx = nxt_record(s);

			incRef(idx);
			return idx;
		}

		size_t RuntimeStorage::setRef(size_t idx, str_record* s) {
			if (!validIndex(idx)) throw std::string{ "Invalid Record Access" };
			return setRef(idx, s->s);
		}

		size_t RuntimeStorage::combine(size_t s1, size_t s2) {
			if (!validIndex(s2)) throw std::string{ "Invalid Record Access" };
			return combine(s1, store[s2]->s);
		}

		size_t RuntimeStorage::combine(size_t s1, std::string s2) {
			if (!validIndex(s1)) throw std::string{ "Invalid Record Access" };
			return setRef(s1, store[s1]->s + s2);
		}

		size_t RuntimeStorage::combine(std::string s1, std::string s2) {
			return loadRef(s1 + s2);
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
			t->s = s;
		}

		void RuntimeStorage::addTemp(str_record* t, size_t s) {
			if (!validIndex(s)) throw std::string{ "Invalid Record Access" };
			addTemp(t, store[s]->s);
		}

		void RuntimeStorage::addTemp(str_record* t, str_record* s) {
			addTemp(t, s->s);
		}

		void RuntimeStorage::addTemp(str_record* t, std::string s) {
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
			return s->s;
		}

		std::string RuntimeStorage::deref(size_t idx) {
			if (!validIndex(idx)) throw std::string{ "Invalid Record Access" };
			return deref(store[idx]);
		}

		int RuntimeStorage::num_records() {
			return store.size() - collected();
		}

		int RuntimeStorage::num_refs(std::string s) {
			return store[registry[s]]->numRefs;
		}

		int RuntimeStorage::collected() {
			return open.size();
		}

		void RuntimeStorage::printAll() {
			for (auto it : registry)
				std::cout << "refs: " << store[it.second]->numRefs << " :: \"" << it.first << "\"\n";
		}


		size_t GC::getIncr() {
			return std::min(c_idx + std::min((size_t)32, lastIndex() / 4), lastIndex());
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