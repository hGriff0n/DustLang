#pragma once

#include "function_traits.h"

namespace shl {

	/*
	* Check if the Callable type has the given Signature
	* Fails to compile when not given a Callable for the first template
	*/
	template <typename Callable, typename Signature>
	struct has_signature {
		static constexpr bool value = false;
	};

	// Match lambda and std::function
	template <typename F, typename Ret, typename... Args>
	struct has_signature<F, Ret(Args...)> {
		private:
			template <bool> static constexpr bool has() { return std::is_same<std::result_of_t<F(Args...)>, Ret>::value; }
			template <> static constexpr bool has<false>() { return false; }

		public:
			static constexpr bool value = has<function_traits<F>::arity == sizeof...(Args)
											&& std::is_same<function_traits<F>::arg_types, function_traits<Ret(Args...)>::arg_types>::value>();
	};

	// Match raw functions
	template <typename R1, typename... Args1, typename R2, typename... Args2>
	struct has_signature<R1(Args1...), R2(Args2...)> {
		static constexpr bool value = false;
	};
	template <typename Ret, typename... Args>
	struct has_signature<Ret(Args...), Ret(Args...)> {
		static constexpr bool value = true;
	};


	/*
	* Check if the given type is Callable (operator() is defined)
	*/

	// Match lambdas and std::function
		// Doesn't work with generic lambdas
	template <typename F>
	struct is_callable {
		private:
			using Yes = char;
			using No = long;

			template <typename T> static constexpr Yes is(decltype(&T::operator()));
			template <typename T> static constexpr No is(...);

		public:
			static constexpr bool value = (sizeof(is<F>(nullptr)) == sizeof(Yes));
	};

	// Match raw function
	template <typename Ret, typename... Args>
	struct is_callable<Ret(Args...)> {
		static constexpr bool value = true;
	};

	// Match function pointer
	template <typename Ret, typename... Args>
	struct is_callable<Ret(*)(Args...)> {
		static constexpr bool value = true;
	};


	/*
	 * Check if the given type meets the expected functional interface
	 * Handles non-function types gracefully (as opposed to has_signature)
	 */
	template <typename Fn, typename Sig>
	struct has_interface {
		private:
			template<bool> static constexpr bool has() { return false; }
			template<> static constexpr bool has<true>() { return has_signature<Fn, Sig>::value; }

		public:
			static constexpr bool value = has<is_callable<Fn>::value>();
	};

}
