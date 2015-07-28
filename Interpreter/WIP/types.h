#pragma once

#include "defines.h"
//#include "type_data.h"

// Think of renaming these classes and functions (maybe easier if I move them into a namespace)


// size: 8
union DustVal {
	int i;
	double d;
	//void* u;

	DustVal() {}
	DustVal(int value) : i{ value } {}
	DustVal(double value) : d{ value } {}
	DustVal(DustVal& copy) {
		*this = copy;
	}
};


// maybe move to type_data
template <typename T>
struct Value {};

template <> struct Value<int> {
	static const ValType type = ValType::INT;

	// I could even generalize these into another class (so far they have the same signatures)
	static int convert(DustVal val, ValType type) {
		switch (type) {
			case ValType::INT:
			case ValType::BOOL:
				return val.i;
			case ValType::FLOAT:
				return val.d;
			default:
				return 0;				// throw error
		}
	}
};

template <> struct Value<double> {
	static const ValType type = ValType::FLOAT;

	static double convert(DustVal val, ValType type) {
		switch (type) {
			case ValType::INT:
			case ValType::BOOL:
				return val.i;
			case ValType::FLOAT:
				return val.d;
			default:
				return 0;				// throw error
		}
	}
};

template <> struct Value<bool> {
	static const ValType type = ValType::BOOL;

	static bool convert(DustVal val, ValType type) {
		switch (type) {
			case ValType::INT:
			case ValType::BOOL:
				return val.i;
			case ValType::FLOAT:
				return val.d;
			default:
				return 0;				// throw error
		}
	}
};


// size: 24
struct DustObj {
	ValType type;			// 4
	DustVal val;			// 8
	bool typed, let;		// 2

	DustObj(ValType t, DustVal v) : DustObj{ t, v, false, false } {}
	DustObj(ValType t, DustVal v, bool s, bool c) : type{ t }, val{ v }, typed{ s }, let{ c } {}

	template <typename T>
	operator T() {
		return Value<T>::convert(val, type);
	}

	template <typename T>
	DustObj& operator=(T val) {
		setObj(*this, val);
		return *this;
	}
};

/*
template <typename T>
DustObj makeObj(T val) {
	return makeObj(val, false, false);
}
*/

template <typename T>			// it might be beneficial to remove the default arguments (constant and not typed). The default is then a no-argument function (or require all args)
DustObj makeObj(T val, bool typed = false, bool constant = false) {
	return DustObj{ Value<T>::type, val, typed, constant };
}

template <typename T>
void setObj(DustObj& o, T val) {
	// if (o.let) throw error
	// else
	{
		// if (o.typed) try conversion (to o.type)
		// else
		{
			o.type = Value<T>::type;
			o.val = val;
		}
	}
}

template <typename T>
void recast(DustObj& o) {
	// if (o.typed) throw error
	// else
	{
		o = (T)o;
	}
}

// Needs:
	// const and static bindings (disallow reassignment/other types)
		// I can start to integrate this into the evaluate here
	// string data types
	// Improve the syntax and api