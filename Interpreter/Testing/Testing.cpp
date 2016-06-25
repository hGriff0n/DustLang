#include "Testing.h"
#include "../libs/catch.hpp"

using namespace std;
using namespace dust;
using namespace dust::test;

// Rename to NO_THROW
#define DO_EVAL(expr) REQUIRE_NOTHROW({ run(e, expr); })
#define MUST_THROW(exception_type, expr) REQUIRE_THROWS_AS({ run(e, expr); }, exception_type)

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

static void run(EvalState& e, const std::string& code) {
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
		DO_EVAL("3");
		REQUIRE(typeof(e) == "Int");
		REQUIRE((int)e == 3);

		DO_EVAL("3.3");
		REQUIRE(typeof(e) == "Float");
		REQUIRE((double)e == 3.3);

		DO_EVAL("\"Hello\"");
		REQUIRE(typeof(e) == "String");
		REQUIRE((string)e == "Hello");

		DO_EVAL("true");
		REQUIRE(typeof(e) == "Bool");
		REQUIRE((bool)e);

		DO_EVAL("false");
		REQUIRE(typeof(e) == "Bool");
		REQUIRE_FALSE((bool)e);

		DO_EVAL("nil");
		REQUIRE(typeof(e) == "Nil");
		e.pop();

		DO_EVAL("[]");
		REQUIRE(typeof(e) == "Table");
		e.pop();

		DO_EVAL("[ 1 2 ]");
		REQUIRE(typeof(e) == "Table");
		e.pop();

		DO_EVAL("[ a: 1 2 ]");
		REQUIRE(typeof(e) == "Table");
		e.pop();
	}

	SECTION("Lambdas", "[!mayfail]") {
		DO_EVAL("\\x -> x");
		REQUIRE(typeof(e) == "Function");
		e.pop();

		// TODO: Add more lambda tests
	}
}

// Note: Move ternary tests here if I use them in any intervening tests
TEST_CASE("Operator Evaluation") {
	EvalState e;
	initState(e);

	SECTION("Unary Operators") {

	}

	SECTION("Binary Operators") {

	}

	SECTION("Boolean Operators") {
		DO_EVAL("true and false");
		REQUIRE_FALSE((bool)e);

		DO_EVAL("true and true");
		REQUIRE((bool)e);

		DO_EVAL("true or false");
		REQUIRE((bool)e);

		DO_EVAL("false or false");
		REQUIRE_FALSE((bool)e);
	}

	SECTION("Operator Resolution") {

	}

	SECTION("Operator Precedence") {

	}
}

TEST_CASE("Variable Assignment") {
	EvalState e;
	initState(e);

	SECTION("Assignment Basics") {

	}

	SECTION("Compound Assignment") {
		run(e, "a: 3");			// temporary setup ??
		run(e, "b: 0");

		DO_EVAL("a, b:+ 2, 2");
		REQUIRE((int)e == 2);
		DO_EVAL("a = 5 and b = 2");
		REQUIRE((bool)e);

		MUST_THROW(error::dispatch_error, "a, b:* 2");
		DO_EVAL("a = 10");
		REQUIRE((bool)e);

		MUST_THROW(error::dispatch_error, "a, b:* nil, 2");
		DO_EVAL("b = 2");
		REQUIRE((bool)e);

		run(e, "a, b: 5, 2");

		DO_EVAL("a, b:* 2, 0");
		REQUIRE((int)e == 0);
		DO_EVAL("a = 10 and b = 0");
		REQUIRE((bool)e);

		DO_EVAL("a:= (b: 3) * 2 + 2 ^ 2");
		REQUIRE((bool)e);
		DO_EVAL("a and b = 3");
		REQUIRE((bool)e);
	}

	SECTION("Boolean Shortcircuiting") {

	}
}

// Consider rearranging some of these test cases
TEST_CASE("Tables") {
	EvalState e;
	initState(e);

	SECTION("Indexing") {
		run(e, "a, b: [ 1 ], 1");
		DO_EVAL("a.1");
		REQUIRE((int)e == 1);
		DO_EVAL("a.1 = a[b]");
		REQUIRE((bool)e);

		run(e, "a.a: [ a: 3 ]");
		DO_EVAL("a"); // -> "[1, a: [ a: 3 ] ]"
		REQUIRE((string)e == "[ 1, a: [ a: 3 ] ]");
		DO_EVAL("a.a"); // -> "[ a: 3 ]"
		REQUIRE((string)e == "[ a: 3 ]");
		DO_EVAL("a.a.b: 2"); // -> 2
		REQUIRE((int)e == 2);
		DO_EVAL("a.a"); // -> "[ a: 3, b: 2 ]"
		REQUIRE((string)e == "[ a: 3, b: 2 ]");

		// I don't test multiple expressions or commas
	}

	SECTION("Operators") {
		run(e, "a: [ 1 2 3 2 5 5 4 ]");
		run(e, "b: [ 1 3 ]");
		run(e, "c: 5");

		DO_EVAL("(a ^ b) = b");
		REQUIRE((bool)e);
		DO_EVAL("a ^ c"); // -> "[ 5, 5 ]"
		REQUIRE((string)e == "[ 5, 5 ]");
		DO_EVAL("b + c"); // -> "[ 1, 3, 5 ]"
		REQUIRE((string)e == "[ 1, 3, 5 ]");
		DO_EVAL("a - b"); // -> "[ 2, 2, 5, 5, 4 ]"
		REQUIRE((string)e == "[ 2, 2, 5, 5, 4 ]");
		DO_EVAL("b * c"); // -> "[ 1, 3, 5 ]"
		REQUIRE((string)e == "[ 1, 3, 5 ]");
	}

	SECTION("Advanced") {
		DO_EVAL("a[b[2]]");
		REQUIRE((int)e == 3);			// ERROR: Couldn't convert to Int in Traits<int>::get
		DO_EVAL("a[b[2] * 2]");
		REQUIRE((int)e == 5);
	}

	SECTION("Experimental", "[!mayfail]") {
		DO_EVAL("[2].1");
		REQUIRE((int)e == 2);
	}
}

TEST_CASE("Advanced Parsing") {
	EvalState e;
	initState(e);

	SECTION("Comments") {

	}

	SECTION("Multiline Input") {
		run(e, "a, b: 4, 3");			// temporary setup ???

		DO_EVAL("a\n+ b");
		REQUIRE((int)e == 7);
		DO_EVAL("3 - \n 3");
		REQUIRE((int)e == 0);
		DO_EVAL("3 + 3\n  \n4 + 4");
		REQUIRE((int)e == 8);
		DO_EVAL("## Hello\n3");
		REQUIRE((int)e == 3);
	}

	// Assignment, etc.
	SECTION("Scopes") {
		DO_EVAL("a: 2\n	a:5\n	a");
		REQUIRE((int)e == 5);
		DO_EVAL("a = 2");
		REQUIRE((bool)e);
		DO_EVAL("a: 3\n	a + 3");
		REQUIRE((int)e == 6);
		
		DO_EVAL("a: 4\n"
			"	a: 3\n"
			"	b: a + .a");
		REQUIRE((int)e == 7);
		DO_EVAL(".a");
		REQUIRE((int)e == 4);

		DO_EVAL("a: 2 ## Assign 2 to a\n"
			"	b: .a + 3 ## Assign a + 3 to b\n"
			"	b + a");
		REQUIRE((int)e == 7);

		DO_EVAL("a: 2\n"
			"	b: 3 + .a\n"
			"## Assign 3 + a to b"
			"	b + a");
		REQUIRE((int)e == 7);		// ERROR: 5 == 7

		DO_EVAL("af: 3\n"
			"		5\n"
			"	af");
		REQUIRE((int)e == 3);

		// I don't actually test that the scoped value overrides the global value
	}
}

// Test that I can reliably throw exceptions  from dust code
TEST_CASE("Exceptions") {
	EvalState e;
	initState(e);
}

TEST_CASE("Control Flow Constructions") {
	EvalState e;
	initState(e);

	SECTION("While Loop") {
		DO_EVAL("i: 0\n"
			"while i < 5\n"
			"	.i:+ 1\n"
			"i");
		REQUIRE((int)e == 5);

		DO_EVAL("i: 0\n"
			"while i < 5 .i:+ 1\n");
		REQUIRE((int)e == 5);

		DO_EVAL("i: 0\n"
			"while i < 5 do\n"
			"	.i:+ 1\n");
		REQUIRE((int)e == 5);

		DO_EVAL("i: 5\n"
			"while i < 5 .i:+ 1\n"); // -> nil
		REQUIRE(e.pop().type_id == type::Traits<Nil>::id);
	}

	SECTION("Do-While Loop") {
		DO_EVAL("i: 0\n"
			"repeat i < 5\n"
			"	.i:+ 1\n"
			"i");
		REQUIRE((int)e == 5);

		DO_EVAL("i: 0\n"
			"repeat i < 5 .i:+ 1");
		REQUIRE((int)e == 5);

		DO_EVAL("i: 5\n"
			"repeat i < 5 .i:+ 1");
		REQUIRE((int)e == 6);
	}

	SECTION("For Loop") {
		DO_EVAL("sum: 0\n"
			"for i in [ 1 2 3 4 5 ]\n"
			"	.sum:+ i\n"
			"sum");
		REQUIRE((int)e == 15);

		DO_EVAL("for i in [1]"); // -> nil
		REQUIRE(e.pop().type_id == type::Traits<Nil>::id);

		DO_EVAL("msg: \"\"\n"
			"for w in [ \"Hello,\" \"World!\" \"I'm\" \"Margaret\" ]\n"
			"	.msg:+ w + \" \"\n"
			"msg");
		REQUIRE((string)e == "Hello, World! I'm Margaret ");

		run(e, "a: [ 1 2 b: [ 3 4 5 ] ]");
		DO_EVAL("f: 1\n"
			"for i in a.b .f:* i\n");
		REQUIRE((int)e == 60);

		run(e, "a: [ 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 ]");
		DO_EVAL("sum_a, sum_b: 0, 0\n"
			"for i in (a ^ [ 1 2 3 5 7 11 13 17 ])\n"
			"	.sum_a:+ i\n"
			"for i in [ 1 2 3 5 7 11 13 17 ]\n"
			"	.sum_b:+ i\n"
			"sum_a = sum_b");
		REQUIRE((bool)e);
	}

	SECTION("If Statement") {
		DO_EVAL("if i 3");
		REQUIRE((int)e == 3);		// ERROR: Couldn't convert to Int in Traits<int>::get
		DO_EVAL("if i = 3 6");
		REQUIRE_FALSE((bool)e);
		DO_EVAL("if i = 3 6\n"
			"else 7");
		REQUIRE((int)e == 7);

		DO_EVAL("if i = 3 6\n"
			"elseif i = 4 7\n"
			"else 8");
		REQUIRE((int)e == 8);

		MUST_THROW(error::missing_node_x, "else \"Hello\"");
		MUST_THROW(error::missing_node_x, "if i 3\n"
										  "i: 3 + 3\n"
										  "else 5");
		MUST_THROW(error::invalid_ast_construction, "if i 3\n"
													"else 5\n"
													"elseif a = 3 .i: 6");
	}

	SECTION("Try-Catch Block") {
		DO_EVAL("try\n"
			"	3 + true\n"
			"catch (e) 4\n");
		REQUIRE((int)e == 4);

		DO_EVAL("try 3 + true\n"
			"catch (e) 4");
		REQUIRE((int)e == 4);

		DO_EVAL("try 3 + 3\n"
			"catch (e) 4");
		REQUIRE((int)e == 6);

		DO_EVAL("a: 3\n"
			"try\n"
			"	3 + true\n"
			"	.a: 2\n"
			"catch (e)\n"
			"a");
		REQUIRE((int)e == 3);

		DO_EVAL("a: 3\n"
			"try\n"
			"	3 + 3"
			"	.a: 2\n"
			"catch (e)\n"
			"a");					// ERROR: eolf exception in evaluation
		REQUIRE((int)e == 2);

		DO_EVAL("try\n"
			"	.a: 3 + 3\n"
			"	3 + a\n"
			"catch(e)\n");
		REQUIRE((int)e == 9);

		DO_EVAL("(try 3 + true\n"
			"catch (e) e )<- String");
		REQUIRE((bool)e);
		
		MUST_THROW(error::missing_node_x, "catch (e) 4");
	}
}

// TODO: Figure out what should go in here
TEST_CASE("Type System Basics") {
	EvalState e;
	initState(e);

	SECTION("Type Checking") {
		DO_EVAL("3 <- Int");
		REQUIRE((bool)e);
		DO_EVAL("3 + 0.3 <- Float");
		REQUIRE((bool)e);
	}

	SECTION("Meta-values") {

	}
}

TEST_CASE("Functions") {
	EvalState e;
	initState(e);

	DO_EVAL("abs <- Function");

	SECTION("Calling Functions") {
		DO_EVAL("abs(1)");			// ERROR: Attempt to call a non-function
		REQUIRE((int)e == 1);
		DO_EVAL("abs(-1)");
		REQUIRE((int)e == 1);

		DO_EVAL("add(2, 3)");
		REQUIRE((int)e == 5);
		DO_EVAL("add( 1 + 1,3)");
		REQUIRE((int)e == 5);

		DO_EVAL("give5()");
		REQUIRE((int)e == 5);
		DO_EVAL("give5(3)");
		REQUIRE(e.size() == 1);
		REQUIRE((int)e == 5);

		MUST_THROW(error::runtime_error, "add(2)");
		DO_EVAL("max(3, 5)");
		REQUIRE((int)e == 5);
		DO_EVAL("max(3)");
		REQUIRE((int)e == 3);

		MUST_THROW(error::dispatch_error, "foo(3)");

		run(e, "a: -1");
		DO_EVAL("a.abs()");
		REQUIRE((int)e == 1);
		DO_EVAL("-1.abs()");
		REQUIRE((int)e == -1);
		DO_EVAL("(-1).abs()");
		REQUIRE((int)e == 1);
		DO_EVAL("(5 * -2).abs()");
		REQUIRE((int)e == 10);
		DO_EVAL("int.abs(a - 3)");
		REQUIRE((int)e == 4);

		MUST_THROW(error::dispatch_error, "\"Hello\".abs()");
	}

	SECTION("Functions as First-Class Values") {
		run(e, "sba: abs");
		DO_EVAL("sba <- Function");
		REQUIRE((bool)e);			// ERROR: false
		DO_EVAL("sba(-3) = abs(3)");
		REQUIRE((bool)e);

		run(e, "a: [ abs: abs ]");
		DO_EVAL("a.abs(3) = 3.abs()");
		REQUIRE((bool)e);

		run(e, "abs: 5");
		DO_EVAL("abs <- Int");
		REQUIRE((bool)e);
		DO_EVAL("sba: <- Function");
		REQUIRE((bool)e);
	}

	SECTION("\"def\"-syntax") {
		DO_EVAL("def min(x, y)\n"
			"	if x < y x\n"
			"	else y");
		DO_EVAL("min <- Function");
		REQUIRE((bool)e);

		DO_EVAL("min(3, 5)");
		REQUIRE((int)e == 3);
		DO_EVAL("min(3, 5, 7)");
		REQUIRE(e.size() == 1);			// ERROR: 51 == 1
		REQUIRE((int)e == 3);

		MUST_THROW(error::dispatch_error, "min(3)");

		DO_EVAL("def Float.abs(self)\n"
			"	self < 0 and -self or self");
		DO_EVAL("Float.abs <- Function");
		REQUIRE((bool)e);

		DO_EVAL("Float.abs(-3.3)");
		REQUIRE((float)e == 3.3);
		DO_EVAL("(-5.5).abs()");
		REQUIRE((float)e == 5.5);
		DO_EVAL("5.5.abs()");
		REQUIRE((float)e == 5.5);

		DO_EVAL("def Bool._op+(self, o)\n"
			"	self or o");
		DO_EVAL("true + false");
		REQUIRE((bool)e);

		DO_EVAL("def Int.diverge(a)\n"
			"	return a + 1, a - 1");
		DO_EVAL("a, b: Int.diverge(3)");
		REQUIRE((int)e == 4);
		DO_EVAL("a + b");
		REQUIRE((int)e == 6);

		MUST_THROW(error::dispatch_error, "3.diverge()");
	}

	SECTION("Multiple Return Values") {
		DO_EVAL("a, b: bound(3.3)");			// ERROR: Attempt to call a non-function
		REQUIRE((int)e == 4);
		DO_EVAL("a + b");
		REQUIRE((int)e == 7);
		DO_EVAL("add(bound(4.7))");
		REQUIRE((int)e == 9);
	}

	SECTION("Recursion", "[!mayfail]") {
		run(e, "def factorial(n)\n"
			"	if n = 0 return 1\n"		// FOUND IT!!! return doesn't cause the function to stop running (Changing to an if-else moves the error to 591)
			"	n * factorial(n - 1)");

		DO_EVAL("factorial(0)");			// ERROR: Operator _op* not defined for Nil and Int <- This shouldn't be occuring **here** !!!
		REQUIRE((int)e == 1);
		DO_EVAL("factorial(3)");
		REQUIRE((int)e == 6);
	}
}

TEST_CASE("User-defined Types") {
	EvalState e;
	initState(e);

	// TODO: Distribute these tests among the sections
	SECTION("Definition Syntax") {
		DO_EVAL("type NewType [ a: 0 ]");
		DO_EVAL("foo: NewType.new()");
		DO_EVAL("foo <- NewType");
		REQUIRE((bool)e);

		DO_EVAL("foo.class");
		REQUIRE((string)e == "NewType");
		DO_EVAL("foo.__type = NewType");
		REQUIRE((bool)e);

		DO_EVAL("foo.a");
		REQUIRE((int)e == 0);
		DO_EVAL("foo.a: 3");
		REQUIRE((int)e == 3);
		DO_EVAL("foo.a - NewType.a");
		REQUIRE((int)e == 3);
		DO_EVAL("foo.__type.a != foo.a");
		REQUIRE((bool)e);
		MUST_THROW(error::dust_error, "foo._max: 3");
		
		DO_EVAL("baz: foo.copy()");
		DO_EVAL("baz.a:+ 2");
		REQUIRE((int)e == 5);
		DO_EVAL("baz.a != foo.a");
		REQUIRE((bool)e);

		DO_EVAL("def NewType.new(self, a)\n"
			"	self.a: a or 0");
		DO_EVAL("bar: NewType.new(5) <- NewType");		// ERROR: No function present
		REQUIRE((bool)e);
		DO_EVAL("bar.a");
		REQUIRE((int)e == 5);
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

	// Some of these don't seem like they're in the correct section
	SECTION("Ambiguous Parses", "[!mayfail]") {
		DO_EVAL("3 + (a: 4)");
		REQUIRE((int)e == 7);
		DO_EVAL("a = 4");
		REQUIRE((bool)e);

		MUST_THROW(pegtl::parse_error, "3 + a: 3");

		DO_EVAL("a, b: 3, (c: 4)");
		REQUIRE((int)e == 4);
		DO_EVAL("a = 3 and c = 4");
		REQUIRE((bool)e);

		DO_EVAL("3 +-3");
		REQUIRE((int)e == 0);

		run(e, "a: 2");
		DO_EVAL("(a: 3) + 3 * a");
		REQUIRE((int)e == 12);
		DO_EVAL("a = 3");
		REQUIRE((bool)e);
	}

	SECTION("Ternary Statement") {
		DO_EVAL("true and false or true");
		REQUIRE((bool)e);
		DO_EVAL("!c and 4 or 5");
		REQUIRE((int)e == 4);

		// This is testing boolean shortcircuitting
		MUST_THROW(pegtl::parse_error, "false and a: 5");
		DO_EVAL("false and (a: 5)");
		REQUIRE_FALSE((bool)e);
		DO_EVAL("a = true");
		REQUIRE((bool)e);				// ERROR: false

		DO_EVAL("a: b or 5");
		REQUIRE((int)e == 3);
		DO_EVAL("a = 3");
		REQUIRE((bool)e);

		DO_EVAL("b: c and 4 or (c: 5)");
		REQUIRE((int)e == 5);
		DO_EVAL("b = 5 and c = b");
		REQUIRE((bool)e);

		DO_EVAL("b: c and 4 or (c: 6)");
		REQUIRE((int)e == 4);
		DO_EVAL("b = 4 and c = 5");
		REQUIRE((bool)e);
	}
	
	// TODO: Add
}