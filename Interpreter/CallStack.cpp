#include "CallStack.h"
#include <algorithm>

namespace dust {
	namespace impl {

		CallStack::CallStack(impl::GC& g) : gc{ g } {}

		void CallStack::try_incRef(const impl::Value& val) {
			if (val.type_id == type::Traits<std::string>::id)
				gc.getStrings().incRef(val.val.i);

			if (val.type_id == type::Traits<dust::Table>::id)
				gc.getTables().incRef(val.val.i);

			if (val.type_id == type::Traits<Function>::id)
				gc.getFunctions().incRef(val.val.i);
		}

		void CallStack::try_decRef(const impl::Value& val) {
			if (val.type_id == type::Traits<std::string>::id)
				gc.getStrings().decRef(val.val.i);

			if (val.type_id == type::Traits<dust::Table>::id)
				gc.getTables().decRef(val.val.i);

			if (val.type_id == type::Traits<Function>::id)
				gc.getFunctions().decRef(val.val.i);
		}

		size_t CallStack::setMinSize(size_t new_min) {
			if (new_min > Stack::size()) new_min = 0;

			auto ret = min_size;
			min_size = new_min;
			return ret;
		}

		int CallStack::normalize(int& idx) {
			if (idx >= 0) return idx += min_size;
			
			return Stack::normalize(idx);
		}
		
		bool CallStack::invalidIndex(int idx) {
			return idx < min_size || Stack::invalidIndex(idx);
		}

		std::vector<impl::Value>::iterator CallStack::__begin() {
			return Stack::__begin() + min_size;
		}

		void CallStack::push(const char* val) {
			push<std::string>(val);
		}

		void CallStack::push(const impl::Value& val) {
			try_incRef(val);

			Stack::push(val);
		}

		void CallStack::pushNil() {
			push<Nil>({});
		}

		impl::Value CallStack::pop(int idx) {
			if (empty()) throw error::runtime_error{ "Used up allotted stack space" };

			try_decRef(at(idx));

			return Stack::pop(idx);
		}

		void CallStack::copy(int idx) {
			push(at(idx));
		}

		void CallStack::settop(int idx) {
			size_t loc = std::max(normalize(idx), 0);

			// PushNil if stack is smaller than idx
			if (loc > size())
				while (size() < loc) pushNil();

			// Pop if stack is bigger than idx
			else
				while (size() > loc) pop();
		}

		void CallStack::replace(int idx) {
			auto& v = Stack::at(idx);

			try_decRef(v);
			try_incRef(v = pop());
		}

		bool CallStack::empty() {
			return Stack::size() == min_size;
		}

		size_t CallStack::size() {
			return Stack::size() - min_size;
		}

	}
}