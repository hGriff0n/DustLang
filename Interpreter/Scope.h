#pragma once

#include "Value.h"
#include <string>
#include <map>

namespace dust {
	namespace impl {

		class Scope {
			using key_type = std::string;
			using storage_type = std::map<key_type, Variable>;			// Would an unordered_map be better ?

			private:
				storage_type vars;
				// std::map<size_t, std::map<Value, Variable>>
				// std::map<Value, Variable>							Simplest to implement, possible conflicts

			protected:
			public:
				Scope();

				Variable& getVar(const key_type& key);
				void associate(const key_type& key, Variable val);
				bool has(const key_type& key);
				Value getVal(const key_type& key);
				//void associate(impl::Value key, impl::Variable val);
				//auto operator[](impl::Variable key);
		};

	}
}