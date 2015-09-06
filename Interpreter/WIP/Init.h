#pragma once

// Use to declare initialization functions 

#define EVAL

namespace dust {
	class EvalState;

	namespace impl {
		class RuntimeStorage;
		class GC;
	}

	namespace type {
		struct Type;
		class TypeSystem;
	}
}

void initTypeSystem(dust::type::TypeSystem&);
void initConversions(dust::type::TypeSystem&);
void initOperations(dust::type::TypeSystem&);