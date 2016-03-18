#pragma once

#include <string>
#include <utility>

// Code for constructing variant visitors using C++14 concepts
	// Comes from a post in r/cpp

template <typename F, typename T>
concept bool UnaryFunction() {
	return requires(const F& f, const T& t) {
		{ f(t) }
	};
}

template <typename F, typename BaseInner, typename ArgsT>
struct ComposeVariantVisitor {
	struct Inner : BaseInner {
		Inner(ArgsT&& a) : BaseInner(move(a.second)), f_move(a.first) {}
		
		using BaseInner::operator();

		template <typename T> requires UnaryFunction<F, T>()
		auto operator()(const T& t) {
			return f_(t);
		}

		private:
			F f_;
	};

	ComposeVariantVisitor(ArgsT&& args) : m_args(move(args)) { }

	template <typename Fadd>
	auto on(Fadd&& f) {
		return ComposeVariantVisitor<Fadd, Inner, pair<Fadd, ArgsT>>{ make_pair(move(f), move(m_args)) };
	}

	ArgsT m_args;
};

struct EmptyVariantVisitor {
	struct Inner {
		struct detail_t {};

		Inner(nullptr_t) {}

		void operator()(detail_t&) const {}
	};

	template <typename Fadd>
	auto on(Fadd&& f) {
		return ComposeVariantVisitor<Fadd, Inner, pair<Fadd, nullptr_t>>{ make_pair(move(f), nullptr) };
	}
};

EmptyVariantVisitor begin_variant_visitor() {
	return{};
}