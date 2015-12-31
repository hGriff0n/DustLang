#pragma once

#include <string>
#include <unordered_map>

#include "Table.h"
#include "Function.h"
#include "StorageBase.h"
#include "Exceptions\runtime.h"

namespace dust {
	namespace impl {

		/*
		 * Class to enable a registry for std::string
		 * Default class performs no work when called
		 *  - Is there a generic way to select the registry specialization ???
		 */
		template <typename Value>
		class RegCollector : public StorageBase {
			protected:
				void removeEntry(const Value& v) {}
				bool hasEntry(const Value& v) { return false; }
				size_t getEntry(const Value& v) { return -1; }
				size_t newEntry(const Value& v, size_t ref) { return ref; }
				size_t setEntry(const Value& v, size_t ref) { return ref; }
		};

		template <>
		class RegCollector<std::string> : public StorageBase {
			private:
				std::unordered_map<std::string, size_t> registry;

			protected:
				void removeEntry(const std::string& v) {
					registry.erase(v);
				}

				bool hasEntry(const std::string& v) {
					return registry.count(v) > 0;
				}

				size_t getEntry(const std::string& v) {
					return registry[v];
				}

				size_t newEntry(const std::string& v, size_t ref) {
					return setEntry(v, ref);
				}

				size_t setEntry(const std::string& v, size_t ref) {
					return registry[v] = ref;
				}
		};

		/*
		 * Storage "bin" for non-primitive objects for use in dust
		 * Basically transforms Value into size_t (which impl::Value accepts)
		 */
		template <typename Value>
		class Collector : public RegCollector<Value> {
			struct StorageType {
				Value val;
				int num_refs = 0;

				// Expression SFINAE to ensure that pointers get deleted if Value is a pointer type (StorageType owns its pointers)
				template <typename V>
				void deleteValue(std::enable_if_t<std::is_pointer<V>::value>* = nullptr) {
					delete val;
				}
				
				template <typename V>
				void deleteValue(std::enable_if_t<!std::is_pointer<V>::value>* = nullptr) {}

				StorageType(const Value& v) : val{ v } {}
				~StorageType() {
					deleteValue<Value>();
				}
			};

			private:
				std::vector<StorageType*> store;

				// Possibly replace "if !validIndex throw" pattern ???
				void throwIfInvalid(size_t idx) {
					if (!validIndex(idx)) throw error::storage_access_error{ "TRuntimeStorage<T>::throwIfInvalid", idx };
					if (!store[idx]) throw error::storage_access_error{ "Attempt to access null" };
				}

				// Create a new record
				size_t nxtRecord(Value s) {
					size_t alloc = open.empty() ? store.push_back(nullptr), store.size() - 1 : pop();
					store[alloc] = new StorageType{ s };
					return alloc;
				}

			protected:
				// Mark the given index as "free"
				void markFree(size_t idx) {
					removeEntry(store[idx]->val);

					delete store[idx];
					store[idx] = nullptr;

					open.push_back(idx);
				}

				// Bounds checking
				bool validIndex(size_t idx) {
					return idx >= 0 && idx < store.size() && store[idx];
				}

				// Check if the given index can be collected
				bool isCollectableResource(size_t idx) {
					return validIndex(idx) && store[idx]->num_refs < 1;
				}

			public:
				Collector() {}

				// Get the value from the given record
				Value deref(size_t r) {
					if (!validIndex(r)) throw error::storage_access_error{ "TRuntimeStorage<T>::deref", r };

					return store[r]->val;
				}

				// Create a record with the given value
				size_t loadRef(Value s) {
					size_t ref = hasEntry(s) ? getEntry(s) : newEntry(s, nxtRecord(s));
					return ref;
				}

				// Switch record (reuses memory if possible, no need for garbage collection)
				size_t setRef(size_t r, Value s) {
					decRef(r);

					// Attempt to look into the registry
					if (hasEntry(s))
						r = getEntry(s);
					
					// Reuse the memory if possible
					else if (store[r]->num_refs == 0) {
						removeEntry(store[r]->val);
						store[setEntry(s, r)]->val = s;

					// Create a new record
					} else
						newEntry(s, r = nxtRecord(s));

					store[r]->num_refs++;
					return r;
				}

				size_t setRef(size_t r, size_t ref) {
					if (!validIndex(ref)) throw error::storage_access_error{ "TRuntimeStorage<T>::setRef", ref };
					return setRef(r, store[ref]->val);
				}


				// Garbage Collection interface
				void incRef(size_t r) {
					if (!validIndex(r)) throw error::storage_access_error{ "TRuntimeStorage<T>::incRef", r };
					store[r]->num_refs++;
				}

				void decRef(size_t r) {
					if (!validIndex(r)) throw error::storage_access_error{ "TRuntimeStorage<T>::decRef", r };
					store[r]->num_refs--;
				}


				// Size queries
				size_t capacity() {
					return store.size();
				}

				int numRefs(size_t r) {
					return validIndex(r) ? store[r]->num_refs : 0;
				}
		};

		using StringStorage = Collector<std::string>;
		using TableStorage = Collector<dust::Table>;
		using FuncStorage = Collector<dust::Function>;

	}
}