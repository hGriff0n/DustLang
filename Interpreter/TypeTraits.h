#pragma once

#include "Value.h"
//#include "GC.h"				// Wouldn't need to include <string>
#include <string>

#include "Exceptions\logic.h"

// type::Traits is currently only referenced in CallStack.h, Stack.h (id only), and testing.cpp
	// Should I include "GC.h" here I can move the explicit specializations for get and the string specialization for make into this file
	// Should reduce possibilities of future errors (though I still need to find out how the error occurred (for extensibility))

namespace dust {
	namespace impl {
		class GC;
	}

	struct Nil {};

	namespace type {

		template <typename T>
		struct Traits {
			static size_t id;

			static impl::Value make(T v, impl::GC& gc) {
				return{ v, Traits<T>::id };
			}

			static T get(const impl::Value& v, impl::GC& gc) {
				throw error::illegal_template{ "Attempt to use Traits<T>::get on undefined type T" };
			}
		};

		template<typename T> size_t Traits<T>::id = -1;
		template<> size_t Traits<Nil>::id = 0;
		template<> impl::Value Traits<Nil>::make(Nil v, impl::GC& gc) {
			return{ 0, Traits<Nil>::id };
		}
	}
}
