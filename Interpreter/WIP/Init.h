#pragma once

// Use to declare initialization functions 

namespace dust {
	class EvalState;

	namespace impl {
		struct Type;
		class TypeSystem;
		class RuntimeStorage;
		class GC;
	}
}

void initTypeSystem(dust::impl::TypeSystem&);
void initConversions(dust::impl::TypeSystem&);
void initOperations(dust::impl::TypeSystem&);