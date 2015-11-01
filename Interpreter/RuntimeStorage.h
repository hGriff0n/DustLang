#pragma once

#include <vector>
#include <unordered_map>

namespace dust {
	namespace impl {

		/*
		template <typename s, typename k = s>
		class RuntimeStorage {
			private:
			protected:
				std::vector<s> store;
				std::unordered_map<k, size_t> registry;
				std::vector<size_t> open;
			...
		}
		*/

		class RuntimeStorage {
			protected:
				std::vector<size_t> open;

				size_t pop();

				virtual void markFree(size_t) =0;
				virtual bool validIndex(size_t) =0;
				virtual bool isCollectableResource(size_t) =0;

			public:
				RuntimeStorage();

				int numRecords();
				int collected();
				void tryMarkFree(size_t ref);

				virtual void incRef(size_t ref) =0;
				virtual void decRef(size_t ref) =0;
				virtual size_t size() =0;
				virtual int num_refs(size_t ref) =0;
		};

	}
}