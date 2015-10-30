#include "CallStack.h"
#include <algorithm>

namespace dust {
	namespace impl {

		CallStack::CallStack(impl::GC& g) : gc{ g } {}

		void CallStack::try_incRef(impl::Value& val) {
			if (val.type_id == type::Traits<std::string>::id) gc.getStrings().incRef(val.val.i);
		}

		void CallStack::try_decRef(impl::Value& val) {
			if (val.type_id == type::Traits<std::string>::id) gc.getStrings().decRef(val.val.i);
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
			push(Stack::at(idx));
		}

		void CallStack::settop(int idx) {
			idx = (size_t)std::max(normalize(idx), 0);

			if (idx > size())
				while (size() < idx)
					push(0);

			else
				for (int i = size(); i > idx; --i) {
					try_decRef(at());
					pop();
				}
		}

		void CallStack::replace(int idx) {
			auto& v = Stack::at(idx);

			try_decRef(v);

			v = pop();
		}

	}
}
