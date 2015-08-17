#pragma once

namespace dust {
	namespace impl {
		union Atom {
			int i;
			double d;
			void* u;

			Atom(int v) : i{ v } {}
			Atom(double v) : d{ v } {}
			Atom(void* v) : u{ v }{}
			Atom(Atom& copy) {
				*this = copy;
			}
		};

		struct Value {
			Atom val;
			size_t type_id;

			Value(Atom v, size_t t) : val{ v }, type_id{ t } {}
			Value(const Value& v) : val{ v.val }, type_id{ v.type_id } {}
		};

		struct Variable {
			Value val;
			bool is_const;
			size_t type_id;			// == NIL unless statically typed

			Variable(Value v, size_t t, bool c) : type_id{ t }, val{ v }, is_const{ c } {}
		};
	}
}