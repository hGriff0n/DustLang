#pragma once

#include "Value.h"
#include <string>

#include "Exceptions\logic.h"

// type::Traits is currently only referenced in CallStack.h, Stack.h (id only), and testing.cpp
	// Should I include "DualGC.h" here I can move the explicit specializations for get and the string specialization for make into this file
	// Should reduce possibilities of future errors (though I still need to find out how the error occurred (for extensibility))

namespace dust {
	struct Nil {};

	namespace impl {
		class GC;
	}

	namespace type {

		/*
		 * Traits is a basic type traits struct that abstracts the process
		 * Of creating and accessing impl::Values for any type
		 */
		template <typename T, typename = void>
		struct Traits {
			static size_t id;

			static impl::Value make(const T& v, impl::GC& gc) {
				return{ v, Traits<T>::id };
			}

			static T get(const impl::Value& v, impl::GC& gc) {
				throw error::illegal_template("Traits<T>::get is not implemented for type ", typeid(T));
			}
		};

		// Default id definition
		template<typename T, typename v> size_t Traits<T, v>::id = -1;
		//template<typename T> size_t Traits<T>::id = -1;

		// Nil type specializations
		template<> size_t Traits<Nil>::id = 0;
		template<> impl::Value Traits<Nil>::make(const Nil& v, impl::GC& gc) {
			return{ 0, Traits<Nil>::id };
		}
	}
}
