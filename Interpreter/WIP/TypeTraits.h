#pragma once

#include "Value.h"
#include <string>

namespace dust {
	namespace impl {
		class GC;
	}
}

template <typename T>
struct TypeTraits{
	static size_t id;

	static dust::impl::Value make(T v, dust::impl::GC& gc) {
		return{ v, TypeTraits<T>::id };
	}

	static T get(const dust::impl::Value&, dust::impl::GC&);
};

template <typename T> size_t TypeTraits<T>::id = -1;
