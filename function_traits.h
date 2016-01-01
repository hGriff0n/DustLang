#pragma once

#include <tuple>

namespace shl {

	/*
	 * type_traits struct for functions and function objects
	 */
	template <class F>
	struct function_traits;

	// Match function pointer
	template <class R, class... Args>
	struct function_traits<R(*)(Args...)> : public function_traits<R(Args...)> {};

	// Match raw function
	template <class R, class... Args>
	struct function_traits<R(Args...)> {
		using return_type = R;
		using arg_types = std::tuple<Args...>;
		using __arg_types = std::tuple<Args...>;

		static constexpr size_t arity = sizeof...(Args);

		template <size_t i>
		struct arg {
			using type = std::tuple_element_t<i, arg_types>;
		};
	};

	// Match member functions
	template <class C, class R, class... Args>
	struct function_traits<R(C::*)(Args...)> : public function_traits<R(C&, Args...)> {
		using arg_types = std::tuple<Args...>;
	};

	template <class C, class R, class... Args>
	struct function_traits<R(C::*)(Args...) const> : public function_traits<R(C&, Args...)> {
		using arg_types = std::tuple<Args...>;
	};

	template <class C, class R>
	struct function_traits<R(C::*)> : public function_traits<R(C&)> {};

	// Match functors
	template <class Callable>
	struct function_traits {
		private:
		using call_type = function_traits<decltype(&Callable::operator())>;

		public:
		using return_type = typename call_type::return_type;
		using arg_types = typename call_type::arg_types;
		using __arg_types = typename call_type::__arg_types;		// Keep the this pointer

		static constexpr size_t arity = call_type::arity - 1;

		template <size_t i>
		struct arg {
			using type = std::tuple_element_t<i + 1, arg_types>;
		};
	};

	// Remove & and && qualifiers
	template <class F>
	struct function_traits<F&> : public function_traits<F> {};

	template <class F>
	struct function_traits<F&&> : public function_traits<F> {};
}