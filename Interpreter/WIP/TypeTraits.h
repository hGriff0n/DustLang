#pragma once

#include "Value.h"
//#include "GC.h"				// Wouldn't need to include <string>
#include <string>

// type::Traits is currently only referenced in CallStack.h, Stack.h (id only), Init.cpp (id only), and testing.cpp
	// Should I include "GC.h" here I can move the explicit specializations for get and the string specialization for make into this file
	// Should reduce possibilities of future errors (though I still need to find out how the error occurred (for extensibility))

// Perhaps move type::Traits into dust::impl

/*/
namespace dust {
	namespace impl {
		class GC;
	}
}

template <typename T>
struct type::Traits{
	// 
	static size_t id;

	// Convert an object of type T into its dust representation
	static dust::impl::Value make(T v, dust::impl::GC& gc) {
		return{ v, type::Traits<T>::id };
	}

	// Create an object of type T from its dust representation
	static T get(const dust::impl::Value& v, dust::impl::GC& gc) {
		 throw std::string{ "Attempt to use get method for undefined type::Traits class" };
	}
};


template <typename T> size_t type::Traits<T>::id = -1;

//*/
namespace dust {
	namespace impl {
		class GC;
	}

	namespace type {
		template <typename T>
		struct Traits {
			static size_t id;

			static impl::Value make(T v, impl::GC& gc) {
				return{ v, Traits<T>::id };
			}

			static T get(const impl::Value& v, impl::GC& gc) {
				throw std::string{ "Attempt to use get method for undefined C++ type" };
			}
		};

		template<typename T> size_t Traits<T>::id = -1;
	}
}
//*/