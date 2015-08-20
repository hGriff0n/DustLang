#include "Storage.h"

#include <vector>
#include <unordered_map>
#include <stack>

#include <iostream>

namespace dust {
	namespace impl {
		// size: 36
		struct str_record {
			int numRefs = 0;
			size_t bin;						// I haven't settled on the use for this yet
			std::string s;
		};


		template <typename T, int MAX_NUM>
		struct bin {
			T* storage, *open;

			bin() {
				storage = new T[MAX_NUM];
				open = storage;
			}

			int num_records() { return open - storage; }
			bool full() { return num_records() == MAX_NUM; }
			T* end() { return storage + MAX_NUM; }
		};

		struct str_bin : bin<str_record, 128> {};


		str_record* init(str_record* r, std::string s, size_t bin) {
			r->s = s;
			r->bin = bin;
			return r;
		}

		static std::vector<str_bin> s_storage;
		static std::unordered_map<std::string, str_record*> registry{};
		static size_t curr = -1;

		str_record* nxt_record() {
			if (s_storage.empty() || s_storage[curr].full()) {
				s_storage.emplace_back();
				++curr;
			}

			return s_storage[curr].open++;
		}

		str_record* loadRef(std::string s) {
			return setRef(nullptr, s);
		}

		// Can I implement set differently by passing a str_record* as the second arg
		// Perhaps I can even move the main brunt of the code here
		str_record* setRef(str_record* r, str_record* s) {
			return setRef(r, s->s);
		}

		str_record* setRef(str_record* r, std::string s) {
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

		// Appends a string in-place if it is the only reference
		str_record* combine(str_record* s1, str_record* s2) {
			if (s1->numRefs == 1) {
				registry.erase(s1->s);
				s1->s += s2->s;
				return registry[s1->s] = s1;

			} else
				return setRef(s1, s1->s + s2->s);
		}

		std::string deref(str_record* r) {
			if (r) return r->s;
			else throw std::string{ "Nullptr passed to deref" };
		}

		void incRef(str_record* r) {
			if (r) r->numRefs += 1;
			else throw std::string{ "Nullptr passed to addRef" };
		}

		void decRef(str_record* r) {
			if (r) r->numRefs -= 1;
			else throw std::string{ "Nullptr passed to delRef" };
		}

		str_record* delRef(str_record* r) {
			decRef(r);

			// If r was the only instance of the string
			if (r->numRefs == 0) {
				registry.erase(r->s);

				// Free the register for future allocations
				if (s_storage[curr].open = r + 1)
					s_storage[curr].open--;

				// This causes errors, however I need to preserve memory
				//delete r;

			}

			return nullptr;
		}

		void num_refs(std::string s) {
			if (registry.count(s) == 0)
				std::cout << "No References";
			else
				std::cout << registry[s]->numRefs;
		}

		int num_bins() {
			return s_storage.size();
		}

		int num_records() {
			int sum = 0;

			for (auto b : s_storage)
				sum += b.num_records();

			return sum;
		}

		void printAll() {
			for (auto it : registry)
				std::cout << "bin: " << it.second->bin << ", refs: " << it.second->numRefs << " :: \"" << it.first << "\"\n";
		}

		/*
		str_record* nxt_record() {
			static const int MAX_STRS = bin<str_record>::MAX_NUM - 1;

			auto bin = open.top().first;
			auto off = open.top().second++;

			if (open.size() > 1) open.pop();					// If the next record was previously collected (ie. not new)
			else if (off == MAX_STRS) {							// If the current bin is now full
				open.top().first = bin + 1;
				open.top().second = 0;
			}

			return s_storage[bin].setMax(off).storage + off;			// Set the end of usable bin memory and return the record (Moves end if the record was allocated)
		}
		*/

#ifdef USE_EXP_TEMPS
		// Able to construct a limited number of temporary registers for calculation purposes (This does allow multiple references to a single text string)
		static bin<str_record, 5> temp;

		str_record* tempRef(std::string s) {
			if (temp.full()) throw std::string{ "Attempt to allocate too many temporaries" };
			setTemp(temp.open, s);
			return temp.open++;
		}

		void setTemp(str_record* r, std::string s) {
			r->s = s;
		}

		void flushTemporaries() {
			temp.open = temp.storage;
		}

#endif

	}
}