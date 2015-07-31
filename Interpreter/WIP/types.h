#pragma once

#include "defines.h"
#include <string>
//#include "type_data.h"
#include <iostream>

// Think of renaming these classes and functions (maybe easier if I move them into a namespace)
	// Also think of splitting into many files (this is getting to be a bit unwieldy and type_traits doesn't need to know all that many details)
// What about passing around DustObj (and have the conversion stuff there ???)
// Can I improve the conversion system (currently heavily reliant on c-style casts)

// size: 8
union DustVal {
	int i;					// long long i;				// sizeof(long long) == sizeof(double)
	double d;
	std::string* s;
	//void* u;

	DustVal() {}
	DustVal(int value) : i{ value } {}
	DustVal(double value) : d{ value } {}
	DustVal(std::string* value) : s{ value } {}
	DustVal(DustVal& copy) {
		*this = copy;
	}
};

// How to store strings ????
	// Can I improve this to be tied to the EvalState (I certainly can tie it to threads)
	// Currently this uses a static array in state.cpp
		// Uses new entries for each entry
		// Very small (size 100)
		// Don't like the general implementation
std::string* store(std::string);
int str_size();


// maybe move to type_data
template <typename T>
struct Value {};

template <> struct Value<int> {
	using val_type = int;
	static const ValType type = ValType::INT;

	// I could even generalize these into another class (so far they have the same signatures)
	static int convert(DustVal val, ValType type) {
		switch (type) {
			case ValType::INT:
			case ValType::BOOL:
				return val.i;
			case ValType::FLOAT:
				return val.d;
			case ValType::STRING:
				return std::stoi(*val.s);
			default:
				return 0;				// throw error
		}
	}

	static int value(int val) { return val; }
};

template <> struct Value<double> {
	using val_type = double;
	static const ValType type = ValType::FLOAT;

	static double convert(DustVal val, ValType type) {
		switch (type) {
			case ValType::INT:
			case ValType::BOOL:
				return val.i;
			case ValType::FLOAT:
				return val.d;
			case ValType::STRING:
				return std::stod(*val.s);
			default:
				return 0;				// throw error
		}
	}

	static double value(double val) { return val; }
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
			case ValType::STRING:
				//return std::stoi(*val.s);			// true -> 1 ????
			default:
				return 0;				// throw error
		}
	}

	static bool value(bool val) { return val; }
};

template <> struct Value<std::string> {
	static const ValType type = ValType::STRING;

	static std::string convert(DustVal val, ValType type) {
		switch (type) {
			case ValType::INT:
				return std::to_string(val.i);
			case ValType::BOOL:
				return val.i ? "true" : "false";
			case ValType::FLOAT:
				return std::to_string(val.d);
			case ValType::STRING:
				return *val.s;
			default:
				return "";
		}
	}

	static std::string* value(std::string val) {
		return store(val);
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


inline ValType lub(DustObj& l, DustObj& r) {
	return static_cast<ValType>(__max(static_cast<int>(l.type), static_cast<int>(r.type)));			// Assumes the typing structure in 
}

/*/
// More generalized (might even change the return type to facilitate evaluation (return the "type table"?))
	// Calling "_op+" -> lub(l, r).call("_op+", l, r)   (the exact syntax needs some work)

inline ValType lub(DustObj& l, DustObj& r) {
	return ValType::TABLE;
}
//*/

template <typename T>
DustObj makeObj(T val) {
	return makeObj(val, false, false);
}

inline DustObj makeObj(DustObj val) {
	return val;
}

template <typename T>
DustObj makeObj(T val, bool typed, bool constant) {
	return DustObj{ Value<T>::type, Value<T>::value(val), typed, constant };
}

// This doesn't work well if (T) or to_string(T) is not defined on val
template <typename T>
DustVal convertTo(ValType type, T val) {
	if (type == Value<T>::type) return val;

	switch (type) {
		case ValType::INT:
		case ValType::BOOL:
			return (int)val;
		case ValType::FLOAT:
			return (double)val;
		case ValType::STRING:
			return store(std::to_string(val));
			//return store("This is a string");
			//return (std::string)val;
		default:
			return 0;
	}
}

template <typename T>
void setObj(DustObj& o, T val) {
	if (o.let) {
		// throw error
	} else {
		if (o.typed)		// what if o.type = type(val) ???
			o.val = convertTo(o.type, val);
		else {
			o.type = Value<T>::type;
			o.val = Value<T>::value(val);
		}
	}
}

template <typename T>
void recast(DustObj& o) {
	if (o.typed) {
		// throw error
	} else {
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
		case ValType::STRING:
			out << *val.val.s; break;
		default:
			break;
	}

	return out;
}

// Needs:
	// string data type
		// Improve the storage method
	// Improve basic functionality ???
		// Make the "type" data a bit more fluid
			// This has a lot of reprecussions
			// This is a precursor of the type heirarchy
		// Setup the framework to consider metamethods
		// Further generalize ???
	// Improve the syntax, api, and capabilities
		// steps 4-7 on ToDo
	// Eventually move this system to the type_traits style in "type_traits.h" and similar to autoLua
		// rework and improve the system while doing this