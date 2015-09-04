#pragma once

#include "Value.h"
//#include "GC.h"				// Wouldn't need to include <string>
#include <string>

// TypeTraits is currently only referenced in CallStack.h, Stack.h (id only), Init.cpp (id only), and testing.cpp
	// Should I include "GC.h" here I can move the explicit specializations for get and the string specialization for make into this file
	// Should reduce possibilities of future errors (though I still need to find out how the error occurred (for extensibility))

// Perhaps move TypeTraits into dust::impl

namespace dust {
	namespace impl {
		class GC;
	}
}

template <typename T>
struct TypeTraits{
	// 
	static size_t id;

	// Convert an object of type T into its dust representation
	static dust::impl::Value make(T v, dust::impl::GC& gc) {
		return{ v, TypeTraits<T>::id };
	}

	// Create an object of type T from its dust representation
	static T get(const dust::impl::Value&, dust::impl::GC&);
};


template <typename T> size_t TypeTraits<T>::id = -1;