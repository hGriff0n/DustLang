#pragma once

namespace dust {
	namespace impl {
		union Value {
			int i;
			double d;
			void* u;

			Value(int v) : i{ v } {}
			Value(double v) : d{ v } {}
			Value(void* v) : u{ v }{}
		};

		struct Variable {
			size_t type_id;			// == NIL unless statically typed
			Value val;
			bool is_const;
		};
	}
}