#pragma once

#include "defines.h"
//#include "type_data.h"

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

struct DustObj {
	ValType type;
	DustVal val;

	DustObj(ValType t, DustVal v) : type{ t }, val{ v } {}

	/*
	template <typename T>
	operator T() {
		return convert<type, T>(val);
	}
	*/

	operator int() {
		if (type == ValType::INT || type == ValType::BOOL) return val.i;
		else return (int)val.d;
	}
	operator double() {
		if (type == ValType::FLOAT) return val.d;
		else return (double)val.i;
	}
	operator bool() { return (int)*this; }
};

// maybe move to type_data
template <typename T>
struct Value {};

template <> struct Value<int> {
	static const ValType type = ValType::INT;
};

template <> struct Value<double> {
	static const ValType type = ValType::FLOAT;
};

template <> struct Value<bool> {
	static const ValType type = ValType::BOOL;
};


template <typename T>
DustObj makeObj(T val) {		// rename (the class too)???
	return DustObj{ Value<T>::type, val };
}

// Needs:
	// More robust conversion system
	// Easy way to get data in any type
	// Easy way to set data in any type (reassignment)
	// const and static bindings (disallow reassignment/other types)
	// string data types