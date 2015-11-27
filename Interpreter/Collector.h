#pragma once

#include <vector>
#include <unordered_map>
#include <string>

#include "Table.h"
#include "Exceptions\runtime.h"

namespace dust {
	namespace impl {
		namespace experimental {

			template <typename Value>
			class Collector {
				struct StorageType {
					Value val;
					int num_refs;

					StorageType Value(const Value& v) : val{ v } {}
				};

				private:
					std::vector<size_t> open;
					std::vector<StorageType*> store;
					std::unordered_map<Value, size_t> registry;

					// Mark the given index as "free"
					void markFree(size_t idx) {
						registry.erase(store[idx]->s);
						open.push_back(idx);

						delete store[idx];
						store[idx] = nullptr;
					}

					// Bounds checking
					bool validIndex(size_t idx) {
						return idx > 0 && idx < store.size();
					}

					// Check if the given index can be collected
					bool isCollectibleResource(size_t idx) {
						return validIndex(idx) && store[idx].num_refs < 1;
					}

					// Possibly replace "if !validIndex throw" pattern ???
					void throwIfInvalid(size_t idx) {
						if (!validIndex(idx)) throw error::storage_access_error{ "TRuntimeStorage<T>::throwIfInvalid", idx };
						if (!store[idx]) throw error::storage_access_error{ "Attempt to access null" };
					}

					// Pop a value from the open stack
					size_t pop() {
						size_t back = open.back();
						open.pop_back();
						return back;
					}

					// Create a new record
					size_t nxtRecord(Value s) {
						size_t alloc = open.empty() ? store.push_back(nullptr), store.size() - 1 : pop();
						store[alloc] = new StorageType{ s };
						return alloc;
					}

				public:
					RuntimeStorage() {}

					// Get the value from the given record
					Value deref(size_t r) {
						if (!validIndex(r)) throw error::storage_access_error{ "TRuntimeStorage<T>::deref", r };
						if (!store[r]) throw error::storage_access_error{ "TRuntimeStorage<T>::deref: No record associated with index" };

						return store[r]->val;
					}

					// Create a record with the 
					size_t loadRef(Value s) {
						size_t ref = (registry.count(s) > 0) ? registry[s] : (registry[s] = nxtRecord(s));
						incRef(ref);
						return ref;
					}

					// Switch record (reuses memory if possible, no need for garbage collection)
					size_t setRef(size_t r, Value s) {
						decRef(r);

						if (registry.count(s) > 0)
							r = registry[s];

						else if (store[r].num_refs == 0) {
							registry.erase(store[r]->val);
							store[registry[s] = r]->val = s;

						} else
							registry[s] = r = nxtRecord(val);

						store[r]->num_refs++;
						return r;
					}

					size_t setRef(size_t r, size_t ref) {
						if (!validIndex(ref)) throw error::storage_access_error{ "TRuntimeStorage<T>::setRef", ref };
						return setRef(r, store[ref]->val);
					}


					// Garbage Collection interface
					void try_markFree(size_t r) {
						if (isCollectibleResource(r)) markFree(r);
					}

					void incRef(size_t r) {
						if (!validIndex(r)) throw error::storage_access_error{ "TRuntimeStorage<T>::incRef", r };
						store[r]->num_refs++;
					}

					void decRef(size_t r) {
						if (!validIndex(r)) throw error::storage_access_error{ "TRuntimeStorage<T>::decRef", r };
						store[r]->num_refs--;
					}


					// Size members
					size_t capacity() {
						return store.size();
					}

					size_t size() {
						return capacity() - numCollected();
					}

					size_t numCollected() {
						return open.size();
					}

					int numRefs(size_t r) {
						return validIndex(r) && store[r] ? store[r]->num_refs : 0;
					}
			};

			typedef Collector<std::string> StringStorage;
			typedef Collector<Table> TableStorage;

		}
	}
}