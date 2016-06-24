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
	return "";
}


TEST_CASE("Literal Parsing") {
	/*
	Int, Float, String, Bool, Table, Lambdas
	*/
}

// Note: Move ternary tests here if I use them in any intervening tests
TEST_CASE("Operator Evaluation") {
	/*
	All operators
	Binary operators
	Operator resolution
	*/
}

TEST_CASE("Tables") {
	/**/
}

TEST_CASE("Variable Assignment") {
	/*
	Compound Assignment
	*/
}

TEST_CASE("Advanced Parsing") {
	/*
	Comments
	Multiline Input
	Scoping <- Assignment, etc.
	*/
}

TEST_CASE("Exceptions") {
	/**/
}

TEST_CASE("Control Flow Constructions") {
	/*
	While
	Do-While
	For
	If
	Try-Catch
	*/
}

// TODO: Figure out what should go in here
TEST_CASE("Type System Basics") {
	/*
	type check operator
	type meta-values
	*/
}

TEST_CASE("Functions") {
	/*
	Calling Free and OOP
	First-Class Values
	"def"-syntax
	Multiple Returns
	Recursion
	*/
}

TEST_CASE("User-defined Types") {
	/*
	w/ Default type-methods
	w/ Custom type-methods
	w/ Static members
	Inheritance
		Inheritance from standard types ???
	*/
}


/***********************************
 * FUTURE TEST CASES TO GO HERE!!! *
 ***********************************/


TEST_CASE("Tricky Evaluations") {
	/*
	Possibly ambiguous parses
	Ternary construction
	TODO: Add
	*/
}