#include "Testing.h"
#include "../libs/catch.hpp"

using namespace std;
using namespace dust;
using namespace dust::test;

// Helper for figuring out the various capabilities and tools of Catch (ie. how to do X)
TEST_CASE("CATCH Capability Testing", "[.]") {
	SECTION("REQUIRE AND CHECK") {
		// CHECK(expr) - The expression evaluats to true
		// REQUIRE(expr) - The expression evaluates to true. Aborts if false

		// _FALSE(expr) - The expression evaluates to false
	}

	SECTION("FLOATING POINT") {
		// Use Approx class to handle possible floating point errors
		// The epsilon and scale can be modified through class methods
	}

	SECTION("EXCEPTIONS") {
		// _THROWS(expr) - Any exception is thrown
		// _THROWS_AS(expr, excep) - The given exception type is thrown
		// _NOTHROW(expr) - No exceptions are thrown
	}

	// Note: This section is in development on CATCH
	SECTION("MATCHERS") {
		// _THAT(lhs, matcher call) - ???
	}

	SECTION("LOGGING") {
		// INFO(msg) - Log the message if the next assertion fails
		// WARN(msg) - Log the message
		// FAIL(msg) - Log the message and abort
		// CAPTURE(var) - Capture the name and value of a variable
	}

	SECTION("TAGS") {
		// .{} - Exclude unless {} is passed in the command line
		// !throws - Exclude when running with -e/--nothrow
		// !shouldfail - Test is successful if it fails and vice versa
		// !mayfail - Doesn't fail if any assertion fails
	}
}

static void run(EvalState& e, const string& code) {
	parse::AST parse_tree;
	parse::ScopeTracker scp;

	pegtl::parse<grammar, action, parse::control>(code, code, parse_tree, scp);
	parse_tree.pop()->eval(e);
}

static string typeof(EvalState& e) {
	return e.getTS().getName(e.at().type_id);
}


TEST_CASE("Literal Parsing") {
	EvalState e;
	initState(e);
	
	SECTION("Simple Literals") {
		run(e, "3");
		REQUIRE(typeof(e) == "Int");
		REQUIRE((int)e == 3);

		run(e, "3.3");
		REQUIRE(typeof(e) == "Float");
		REQUIRE((double)e == 3.3);

		run(e, "\"Hello\"");
		REQUIRE(typeof(e) == "String");
		REQUIRE((string)e == "Hello");

		run(e, "true");
		REQUIRE(typeof(e) == "Bool");
		REQUIRE((bool)e);

		run(e, "false");
		REQUIRE(typeof(e) == "Bool");
		REQUIRE_FALSE((bool)e);

		run(e, "nil");
		REQUIRE(typeof(e) == "Nil");
		e.pop();

		run(e, "[]");
		REQUIRE(typeof(e) == "Table");
		e.pop();

		run(e, "[ 1 2 ]");
		REQUIRE(typeof(e) == "Table");
		e.pop();

		run(e, "[ a: 1 2 ]");
		REQUIRE(typeof(e) == "Table");
		e.pop();
	}

	SECTION("Lambdas", "[!mayfail]") {
		run(e, "\\x -> x");
		REQUIRE(typeof(e) == "Function");
		e.pop();

		// TODO: Add more lambda tests
	}
}

// Note: Move ternary tests here if I use them in any intervening tests
TEST_CASE("Operator Evaluation") {
	EvalState e;
	initState(e);

	SECTION("Built-in Operators") {

	}

	SECTION("Binary Operators") {

	}

	SECTION("Operator Resolution") {

	}
}

TEST_CASE("Tables") {
	EvalState e;
	initState(e);
}

TEST_CASE("Variable Assignment") {
	EvalState e;
	initState(e);

	SECTION("Assignment Basics") {

	}

	SECTION("Compound Assignment") {

	}
}

TEST_CASE("Advanced Parsing") {
	EvalState e;
	initState(e);

	SECTION("Comments") {

	}

	SECTION("Multiline Input") {

	}

	// Assignment, etc.
	SECTION("Scopes") {

	}
}

TEST_CASE("Exceptions") {
	EvalState e;
	initState(e);
}

TEST_CASE("Control Flow Constructions") {
	EvalState e;
	initState(e);

	SECTION("While Loop") {

	}

	SECTION("Do-While Loop") {

	}

	SECTION("For Loop") {

	}

	SECTION("If Statement") {

	}

	SECTION("Try-Catch Block") {

	}
}

// TODO: Figure out what should go in here
TEST_CASE("Type System Basics") {
	EvalState e;
	initState(e);

	SECTION("Type Checking") {

	}

	SECTION("Meta-values") {

	}
}

TEST_CASE("Functions") {
	EvalState e;
	initState(e);

	SECTION("Calling Functions") {

	}

	SECTION("Functions as First-Class Values") {

	}

	SECTION("\"def\"-syntax") {

	}

	SECTION("Multiple Return Values") {

	}

	SECTION("Recursion", "[!mayfail]") {

	}
}

TEST_CASE("User-defined Types") {
	EvalState e;
	initState(e);

	SECTION("Definition Syntax") {

	}

	SECTION("Using default type-methods") {

	}

	SECTION("Using custom type-methods") {

	}

	SECTION("Types with static members") {

	}

	SECTION("Inheritance") {

	}
}


/***********************************
 * FUTURE TEST CASES TO GO HERE!!! *
 ***********************************/


TEST_CASE("Tricky Evaluations") {
	EvalState e;
	initState(e);

	SECTION("Ambiguous Parses", "[!mayfail]") {

	}

	SECTION("Ternary Statement") {

	}
	
	// TODO: Add
}