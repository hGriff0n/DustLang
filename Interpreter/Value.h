#pragma once


namespace dust {
	namespace impl {

		/*
		 * Raw data structure for implementing dynamic typing
		 *		Convert to variant ???
		 * Holds an int, double, or void*
		 */
		union Atom {
			int i;				// 4
			double d;			// 8
			void* u;			// 4

			Atom() {}
			Atom(int v) : d{} { i = v; }
			Atom(double v) : d{ v } {}
			Atom(size_t v) : d{} { i = v; }
			Atom(void* v) : d{} { u = v; }
			Atom(const Atom& copy) {
				*this = copy;
			}
		};

		/*
		Atom type
		INT		|	i
		BOOL	|	i
		STRING	|	i
		TABLE	|	i
		FLOAT	|	d
		...
		*/


		/*
		 * Structure for holding a singular dust value and type information
		 */
		struct Value {
			Atom val;
			//char idu;
			size_t type_id;
			bool object = false;

			Value() { type_id = 0; }
			Value(Atom v, size_t t) : val{ v }, type_id{ t } {}
			Value(const Value& v) : val{ v.val }, type_id{ v.type_id } {}

			operator size_t() { return type_id; }

			friend bool operator<(const Value& lhs, const Value& rhs) {
				if (lhs.type_id != rhs.type_id) return lhs.type_id < rhs.type_id;				// Doesn't force ints to be lowest (Int::id = 3, only Number, Object and Nil are lower)

				return lhs.val.d < rhs.val.d;
			}

			friend bool operator==(const Value& lhs, const Value& rhs) {
				return lhs.type_id == rhs.type_id && lhs.val.d == rhs.val.d;
			}
		};

		/*
		 * Structure for holding a singular dust variable and type information
		 *	A variable consists of a value that may be of a more specific type than the variable
		 */
		struct Variable {
			Value val;
			bool is_const = false;
			size_t type_id;			// == NIL unless statically typed

			Variable() { type_id = 0; }
			Variable(Value v, size_t t, bool c) : type_id{ t }, val{ v }, is_const{ c } {}

		};
	}
}