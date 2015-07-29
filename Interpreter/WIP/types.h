#pragma once

#include "defines.h"
//#include "type_data.h"
#include <iostream>

// Think of renaming these classes and functions (maybe easier if I move them into a namespace)
// What about passing around DustObj (and have the conversion stuff there ???)
// Can I improve the conversion system (currently heavily reliant on c-style casts)

// size: 8
union DustVal {
	int i;
	// long long i;				// sizeof(long long) == sizeof(double)
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

// template <ValType T>
//struct Vals {};				// needs a better name

// template <> struct Vals<ValType::INT> { using type = Value<int>; }
// template <> struct Value<int> { using type = Vals<ValType::INT>; }

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

	DustObj() {}
	DustObj(ValType t, DustVal v) : DustObj{ t, v, false, false } {}
	DustObj(ValType t, DustVal v, bool s, bool c) : type{ t }, val{ v }, typed{ s }, let{ c } {}

	// the 'explicit' allows the operator<< override to be selected but dirties the code (I'm probably going to change this later)
	template <typename T>
	explicit operator T() {
		return Value<T>::convert(val, type);
	}

	template <typename T>
	DustObj& operator=(T val) {
		setObj(*this, val);
		return *this;
	}

	//bool operator==(DustObj& other);		// Quick Table equality??
};

inline ValType commonType(DustObj& l, DustObj& r) {
	return static_cast<ValType>(__max(static_cast<int>(l.type), static_cast<int>(r.type)));			// Assumes the typing structure in 
}

template <typename T>
DustObj makeObj(T val) {
	return makeObj(val, false, false);
}

inline DustObj makeObj(DustObj val) {
	return val;
}

template <typename T>
DustObj makeObj(T val, bool typed, bool constant) {
	return DustObj{ Value<T>::type, val, typed, constant };
}

template <typename T>
DustVal convertTo(ValType type, T val) {
	switch (type) {
		case ValType::INT:
		case ValType::BOOL:
			return (int)val;
		case ValType::FLOAT:
			return (double)val;
		default:
			return 0;
	}
}

template <typename T>
void setObj(DustObj& o, T val) {
	if (o.let)
		0;		// throw error
	else {
		if (o.typed)
			o.val = convertTo(o.type, val);
		else {
			o.type = Value<T>::type;
			o.val = val;
		}
	}
}

template <typename T>
void recast(DustObj& o) {
	if (o.typed)
		0;		 //throw error
	else {
		o = (T)o;
	}
}

template <class stream>
stream& operator<<(stream& out, DustObj val) {
	switch (val.type) {
		case ValType::INT:
		case ValType::BOOL:
			out << val.val.i; break;
		case ValType::FLOAT:
			out << val.val.d; break;
		default:
			break;
	}

	return out;
}

// Needs:
	// Integrate this work into the existing framework (step 1 on ToDo)
	// Improve basic functionality ???
		// Make the "type" data a bit more fluid
			// This has a lot of reprecussions
			// This is a precursor of the type heirarchy
		// Setup the framework to consider metamethods
		// Further generalize ???
	// string data types
	// Improve the syntax, api, and capabilities
		// steps 4-7 on ToDo
	// Eventually move this system to the type_traits style in "type_traits.h" and similar to autoLua
		// rework and improve the system while doing this