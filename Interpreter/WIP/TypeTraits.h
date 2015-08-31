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

	// Is there a way of generalizing this for strings ???
	static dust::impl::Value make(T v, dust::impl::GC& gc) {
		return { v, TypeTraits<T>::id };
	}

	static T get(const dust::impl::Value&, dust::impl::GC&);
};

// std::string has a different implementation of make (currently defined in testing.cpp)
// However the std::specialization for TypeTraits::make is being created somewhere between here and there
	// Giving TypeTraits<std::string> the wrong make function. This alleviates that error at the cost of a C4506 warning
template<> dust::impl::Value TypeTraits<std::string>::make(std::string, dust::impl::GC&);
template <typename T> size_t TypeTraits<T>::id = -1;