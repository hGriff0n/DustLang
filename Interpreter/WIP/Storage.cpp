#include "Storage.h"

#include <unordered_map>
#include <iostream>

namespace dust {
	namespace impl {
		static std::unordered_map<std::string, str_record*> registry{};

#ifndef USE_BIN_STORAGE
		// size: 32
		struct str_record {
			int numRefs = 0;
			std::string s;

			//str_record() {}
			//str_record(std::string str) : s{ str } {}
			//~str_record() {}
		};

		static size_t MAX_STRS = 128;
		static str_record* s_storage = new str_record[MAX_STRS];
		static str_record* s_open = s_storage;

		str_record* nxt_record() {
			// if (!open.empty()) return str_storage + open.pop();
			
			if (num_records() == MAX_STRS) throw std::string{"Reached max number of strings"};

			return s_open++;
		}

		str_record* set(str_record* r, std::string s) {
			// If r is not nullptr (the record already exists
			if (r) {
				// If the record is the only reference, reuse the record
				if (r->numRefs <= 1) {
					registry.erase(r->s);
					(registry[s] = r)->s = s;
					return r;

				// Otherwise decrement the references
				} else
					delRef(r);
			}

			// If the string already exists, get the record
			if (registry.count(s) > 0) {
				r = registry[s];

			// Otherwise make a new record
			} else {
				r= registry[s] = nxt_record();
				r->s = s;
			}

			addRef(r);
			return r;
		}

		void printAll() {
			for (auto it : registry)
				std::cout << it.first << " :: " << it.second->numRefs << std::endl;
		}

		int num_records() {
			return s_open - s_storage;
		}


		int num_bins() { return 1; }

#else

#include <vector>
		struct str_record {
			int numRefs = 0;
			size_t bin;						// I haven't settled on the use for this yet
			std::string s;
		};

		template <typename T>
		struct bin {
			static const int MAX_NUM = 128;
			T* storage, *open;			// Not needed if moving to a stack based system

			bin() {
				storage = new T[MAX_NUM];
				open = storage;
			}

			int num_records() { return open - storage; }
			bool full() { return num_records() == MAX_NUM; }
			T* end() { return storage + MAX_NUM; }
		};

		//std::stack<std::pair<size_t, int>> open{ std::make_pair(0, 0) };			// The lowest item on the stack takes the place of the open field in bin.
		static size_t curr = -1;
		static std::vector<bin<str_record>> s_storage;

		str_record* nxt_record() {
			if (s_storage.empty() || s_storage[curr].full()) {
				s_storage.emplace_back();
				++curr;
			}

			return s_storage[curr].open++;
		}

		str_record* init(str_record* r, std::string s, size_t bin) {
			r->s = s;
			r->bin = bin;
			return r;
		}

		str_record* set(str_record* r, std::string s) {
			if (r) delRef(r);

			// If the string already exists, get the record
			if (registry.count(s) > 0)
				r = registry[s];

			// If the record was the only reference, reuse the record
			else if (r && r->numRefs == 0) {
				registry.erase(r->s);
				init(registry[s] = r, s, r->bin);

			} else	// Otherwise get new record
				r = init(registry[s] = nxt_record(), s, curr);

			addRef(r);
			return r;
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
			auto bin = open.top().first;
			auto offset = open.top().second++;

			if (open.size() > 1) open.pop();								// If this uses a garbage collected reference
			else if (offset == bin<str_record>::MAX_NUM - 1) {				// If the current bin is now full
				open.top().first = bin + 1;
				open.top().second = 0;
			}

			return s_storage[bin].storage + offset;
		}
		/*/

#endif

		str_record* makeRecord(std::string s) {
			return set(nullptr, s);
		}

		// Can I implement set differently by passing a str_record* as the second arg
		// Perhaps I can even move the main brunt of the code here
		str_record* set(str_record* r, str_record* s) {
			return set(r, s->s);
		}

		std::string deref(str_record* r) {
			if (r) return r->s;
			else throw std::string{ "Nullptr passed to deref" };
		}

		void addRef(str_record* r) {
			if (r) r->numRefs += 1;
			else throw std::string{ "Nullptr passed to addRef" };
		}

		void delRef(str_record* r) {
			if (r) r->numRefs -= 1;
			else throw std::string{ "Nullptr passed to delRef" };
		}

		void num_refs(std::string s) {
			if (registry.count(s) == 0)
				std::cout << "No References";
			else
				std::cout << registry[s]->numRefs;
		}
	}
}