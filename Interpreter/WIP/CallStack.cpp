#include "CallStack.h"

namespace dust {

	CallStack::CallStack(impl::GC& g) : gc{ g } {}

	void CallStack::push(const char* val) {
		push<std::string>(val);
	}

	void CallStack::push(impl::Value val) {
		if (val.type_id == TypeTraits<std::string>::id) gc.incRef(val.val.i);

		Stack::push(val);
	}

	impl::Value CallStack::pop(int idx) {
		return Stack::pop(idx);
	}

	void CallStack::copy(int idx) {
		push(Stack::at(idx));
	}

	void CallStack::settop(int idx) {
		for (int i = size(); i > normalize(idx); --i) {
			if (at().type_id == TypeTraits<std::string>::id) gc.decRef(at().val.i);
			pop();
		}
	}

	void CallStack::replace(int idx) {
		auto& v = Stack::at(idx);

		if (v.type_id == TypeTraits<std::string>::id) gc.decRef(v.val.i);

		v = pop();
	}

	size_t CallStack::pop_ref(bool decRef) {
		if (!is<std::string>(-1)) throw std::logic_error{ "Object at given idx is not a String" };
		if (decRef) gc.decRef(at(-1).val.i);

		return pop(-1).val.i;
	}

}