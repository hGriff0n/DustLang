#include "CallStack.h"
#include <algorithm>

namespace dust {
	namespace impl {

		CallStack::CallStack(impl::GC& g) : gc{ g } {}

		// Handle reference incrementing/decrementing if the type requires it
		void CallStack::try_incRef(impl::Value& val) {
			if (val.type_id == type::Traits<std::string>::id)
				gc.getStrings().incRef(val.val.i);

			if (val.type_id == type::Traits<Table>::id)
				gc.getTables().incRef(val.val.i);
		}

		void CallStack::try_decRef(impl::Value& val) {
			if (val.type_id == type::Traits<std::string>::id)
				gc.getStrings().decRef(val.val.i);

			// This didn't affect the numRefs for tables ???
			if (val.type_id == type::Traits<Table>::id)
				gc.getTables().decRef(val.val.i);
		}

		void CallStack::push(const char* val) {
			push<std::string>(val);
		}

		void CallStack::push(impl::Value val) {
			try_incRef(val);

			Stack::push(val);
		}

		void CallStack::pushNil() {
			push<Nil>({});
		}

		impl::Value CallStack::pop(int idx) {
			try_decRef(at(idx));

			return Stack::pop(idx);
		}

		void CallStack::copy(int idx) {
			push(at(idx));
		}

		void CallStack::settop(int idx) {
			idx = (size_t)std::max(normalize(idx), 0);

			// PushNil if stack is smaller than idx
			if (idx > size())
				while (size() < idx)
					pushNil();

			// Pop if stack is bigger than idx
			else
				for (int i = size(); i > idx; --i)
					pop();
		}

		void CallStack::replace(int idx) {
			auto& v = Stack::at(idx);

			try_decRef(v);
			try_incRef(v = pop());
		}

	}
}
