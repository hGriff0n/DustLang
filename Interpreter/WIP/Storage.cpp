#include "Storage.h"

#include <unordered_map>
#include <iostream>

namespace dust {
	namespace impl {
		
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
		static std::unordered_map<std::string, str_record*> registry{};

		str_record* nxt_record() {
			// if (!open.empty()) return str_storage + open.pop();
			
			if (num_records() == MAX_STRS) throw std::string{"Reached max number of strings"};

			return s_open++;
		}

		str_record* makeRecord(std::string s) {
			return set(nullptr, s);
		}

		// Can I implement set differently by passing a str_record* as the second arg
			// Perhaps I can even move the main brunt of the code here
		str_record* set(str_record* r, str_record* s) {
			return set(r, s->s);
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

		int num_records() {
			return s_open - s_storage;
		}

		void num_refs(std::string s) {
			if (registry.count(s) == 0)
				std::cout << "No References";
			else
				std::cout << registry[s]->numRefs;
		}

		void printAll() {
			for (auto it : registry)
				std::cout << it.first << " :: " << it.second->numRefs << std::endl;
		}


		/*
		static const size_t MAX_USRDATA = 128;
		static usr_record* usr_storage = new usr_record[MAX_USRDATA];
		static usr_record* usr_open = usr_storage;

		void addRef(usr_record* r) {}
		void delRef(usr_record* r) {}
		bool isLoneRef(usr_record* r) {}

		static const size_t MAX_TABLES = 128;
		static tab_record* tab_storage = new tab_record[MAX_TABLES];
		static tab_record* tab_open = tab_storage;

		void addRef(tab_record* r) {}
		void delRef(tab_record* r) {}
		bool isLoneRef(tab_record* r) {}
		*/

	}
}