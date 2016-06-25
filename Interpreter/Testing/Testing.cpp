#include "Testing.h"
#include "../libs/catch.hpp"

using namespace std;
using namespace dust;
using namespace dust::test;

#define DO_EVAL(expr) REQUIRE_NOTHROW({ run(e, expr); })

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
		REQUIRE((bool)e);
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
		DO_EVAL("a, b:+ 2, 2"); // -> 2
		DO_EVAL("a = 5 and b = 2"); // -> true

		// require dispatch_error on "a, b:* 2");
		DO_EVAL("a = 10"); // -> true

		// require dispatch_error on "a, b:* nil, 2");
		DO_EVAL("b = 2"); // -> true

		run(e, "a, b: 5, 2");

		DO_EVAL("a, b:* 2, 0"); // -> 0
		DO_EVAL("a = 10 and b = 0"); // -> true

		DO_EVAL("a:= (b: 3) * 2 + 2 ^ 2"); // -> true
		DO_EVAL("a and b = 3"); // -> true
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
		DO_EVAL("a.1"); // -> 1
		DO_EVAL("a.1 = a[b]"); // -> true

		run(e, "a.a: [ a: 3 ]");
		DO_EVAL("a"); // -> "[1, a: [ a: 3 ] ]"
		DO_EVAL("a.a"); // -> "[ a: 3 ]"
		DO_EVAL("a.a.b: 2"); // -> 2
		DO_EVAL("a.a"); // -> "[ a: 3, b: 2 ]"

		// I don't test multiple expressions or commas
	}

	SECTION("Operators") {
		run(e, "a: [ 1 2 3 2 5 5 4 ]");
		run(e, "b: [ 1 3 ]");
		run(e, "c: 5");

		DO_EVAL("(a ^ b) = b"); // -> true
		DO_EVAL("a ^ c"); // -> "[ 5, 5 ]"
		DO_EVAL("b + c"); // -> "[ 1, 3, 5 ]"
		DO_EVAL("a - b"); // -> "[ 2, 2, 5, 5, 4 ]"
		DO_EVAL("b * c"); // -> "[ 1, 3, 5 ]"
	}

	SECTION("Advanced") {
		DO_EVAL("a[b[2]]"); // -> 3
		DO_EVAL("a[b[2] * 2]"); // -> 5
	}

	SECTION("Experimental", "[!mayfail]") {
		DO_EVAL("[2].1"); // -> 2
	}
}

TEST_CASE("Advanced Parsing") {
	EvalState e;
	initState(e);

	SECTION("Comments") {

	}

	SECTION("Multiline Input") {
		DO_EVAL("a\n+ b"); // -> 7
		DO_EVAL("3 - \n 3"); // -> 0
		DO_EVAL("3 + 3\n  \n4 + 4"); // -> 8
		DO_EVAL("## Hello\n3"); // -> 3
	}

	// Assignment, etc.
	SECTION("Scopes") {
		DO_EVAL("a: 2\n	a:5\n	a"); // -> 5
		DO_EVAL("a = 2"); // -> true
		DO_EVAL("a: 3\n	a + 3"); // -> 6
		
		DO_EVAL("a: 4\n"
			"	a: 3\n"
			"	b: a + .a"); // -> 7
		DO_EVAL(".a"); // -> 4

		DO_EVAL("a: 2 ## Assign 2 to a\n"
			"	b: .a + 3 ## Assign a + 3 to b\n"
			"	b + a"); // -> 7

		DO_EVAL("a: 2\n"
			"	b: 3 + .a\n"
			"## Assign 3 + a to b"
			"	b + a"); // -> 7

		DO_EVAL("af: 3\n"
			"		5\n"
			"	af"); // -> 3

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
			"i"); // -> 5

		DO_EVAL("i: 0\n"
			"while i < 5 .i:+ 1\n"); // -> 5

		DO_EVAL("i: 0\n"
			"while i < 5 do\n"
			"	.i:+ 1\n"); // -> 5

		DO_EVAL("i: 5\n"
			"while i < 5 .i:+ 1\n"); // -> nil
	}

	SECTION("Do-While Loop") {
		DO_EVAL("i: 0\n"
			"repeat i < 5\n"
			"	.i:+ 1\n"
			"i"); // -> 5

		DO_EVAL("i: 0\n"
			"repeat i < 5 .i:+ 1"); // -> 5

		DO_EVAL("i: 5\n"
			"repeat i < 5 .i:+ 1"); // -> 6
	}

	SECTION("For Loop") {
		DO_EVAL("sum: 0\n"
			"for i in [ 1 2 3 4 5 ]\n"
			"	.sum:+ i\n"
			"sum"); // -> 15

		DO_EVAL("for i in [1]"); // -> nil

		DO_EVAL("msg: \"\"\n"
			"for w in [ \"Hello,\" \"World!\" \"I'm\" \"Margaret\" ]\n"
			"	.msg:+ w + \" \"\n"
			"msg"); // -> "Hello, World! I'm Margaret ");

		run(e, "a: [ 1 2 b: [ 3 4 5 ] ]");
		DO_EVAL("f: 1\n"
			"for i in a.b .f:* i\n"); // -> 60

		run(e, "a: [ 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 ]");
		DO_EVAL("sum_a, sum_b: 0, 0\n"
			"for i in (a ^ [ 1 2 3 5 7 11 13 17 ])\n"
			"	.sum_a:+ i\n"
			"for i in [ 1 2 3 5 7 11 13 17 ]\n"
			"	.sum_b:+ i\n"
			"sum_a = sum_b"); // -> true
	}

	SECTION("If Statement") {
		DO_EVAL("if i 3"); // -> 3
		DO_EVAL("if i = 3 6"); // -> false
		DO_EVAL("if i = 3 6\n"
			"else 7"); // -> 7

		DO_EVAL("if i = 3 6\n"
			"elseif i = 4 7\n"
			"else 8"); // -> 8

		// require missing_node_x on "else \"Hello\""
		// require missing_node_x on "if i 3\ni: 3 + 3\nelse 5"
		// require invalid_ast_construction on "if i 3\nelse 5\nelseif a = 3 .i: 6"
	}

	SECTION("Try-Catch Block") {
		DO_EVAL("try\n"
			"	3 + true\n"
			"catch (e) 4\n"); // -> 4

		DO_EVAL("try 3 + true\n"
			"catch (e) 4"); // -> 4

		DO_EVAL("try 3 + 3\n"
			"catch (e) 4"); // -> 6

		DO_EVAL("a: 3\n"
			"try\n"
			"	3 + true\n"
			"	.a: 2\n"
			"catch (e)\n"
			"a"); // -> 3

		DO_EVAL("a: 3\n"
			"try\n"
			"	3 + 3"
			"	.a: 2\n"
			"catch (e)\n"
			"a"); // -> 2

		DO_EVAL("try\n"
			"	.a: 3 + 3\n"
			"	3 + a\n"
			"catch(e)\n"); // -> 9

		DO_EVAL("try 3 + true\n"
			"catch (e) e <- String"); // -> true
		
		// require missing_node_x on "catch (e) 4"
	}
}

// TODO: Figure out what should go in here
TEST_CASE("Type System Basics") {
	EvalState e;
	initState(e);

	SECTION("Type Checking") {
		DO_EVAL("3 <- Int"); // -> true
		DO_EVAL("3 + 0.3 <- Float"); // -> true
	}

	SECTION("Meta-values") {

	}
}

TEST_CASE("Functions") {
	EvalState e;
	initState(e);

	DO_EVAL("abs <- Function"); // true

	SECTION("Calling Functions") {
		DO_EVAL("abs(1)"); // -> 1
		DO_EVAL("abs(-1)"); // -> 1

		DO_EVAL("add(2, 3)"); // -> 5
		DO_EVAL("add( 1 + 1,3)"); // -> 5

		DO_EVAL("give5()"); // -> 5
		DO_EVAL("give5(3)"); // -> 5
		// require size to be 1

		// require runtime_error on "add(2)"
		DO_EVAL("max(3, 5)"); // -> 5
		DO_EVAL("max(3)"); // -> 3

		// require dispatch_error on "foo(3)"

		run(e, "a: -1");
		DO_EVAL("a.abs()"); // -> 1
		DO_EVAL("-1.abs()"); // -> -1
		DO_EVAL("(-1).abs()"); // -> 1
		DO_EVAL("(5 * -2).abs()"); // -> 10
		DO_EVAL("int.abs(a - 3)"); // -> 4

		// require dispatch_error on "\"Hello\".abs()"
	}

	SECTION("Functions as First-Class Values") {
		run(e, "sba: abs");
		DO_EVAL("sba <- Function"); // -> true
		DO_EVAL("sba(-3) = abs(3)"); // -> true

		run(e, "a: [ abs: abs ]");
		DO_EVAL("a.abs(3) = 3.abs()");

		run(e, "abs: 5");
		DO_EVAL("abs <- Int");
		DO_EVAL("sba: <- Function");
	}

	SECTION("\"def\"-syntax") {
		// No error on "def min(x, y)\n\tif x < y x\nelse y"
		DO_EVAL("min <- Function");

		DO_EVAL("min(3, 5)"); // -> 3
		DO_EVAL("min(3, 5, 7)"); // -> 3
		// Stack size after "min(3, 5, 7)" is 1

		// require dispatch_error on "min(3)"

		// No error on "def Float.abs(self)\n\tself < 0 and -self or self"
		DO_EVAL("Float.abs <- Function");

		DO_EVAL("Float.abs(-3.3)"); // -> 3.3
		DO_EVAL("(-5.5).abs()"); //-> 5.5
		DO_EVAL("5.5.abs()"); // -> 5.5

		// No error on "def Bool._op+(self, o)\n\tself or o"
		DO_EVAL("true + false"); // -> true

		// No error on "def Int.diverge(a)\n\treturn a + 1, a - 1"
		DO_EVAL("a, b: Int.diverge(3)"); // 4
		DO_EVAL("a + b"); // 6
		// require dispatch_error on "3.diverge()"
	}

	SECTION("Multiple Return Values") {
		DO_EVAL("a, b: bound(3.3)"); // -> 4
		DO_EVAL("a + b"); // -> 7
		DO_EVAL("add(bound(4.7))"); // -> 9
	}

	SECTION("Recursion", "[!mayfail]") {
		run(e, "def factorial(n)\n"
			"	if n = 0 return 1\n"
			"	n * factorial(n - 1)");

		DO_EVAL("factorial(0)"); // -> 1
		DO_EVAL("factorial(3)"); // -> 6
	}
}

TEST_CASE("User-defined Types") {
	EvalState e;
	initState(e);

	// TODO: Distribute these tests among the sections
	SECTION("Definition Syntax") {
		// No error on "type NewType [ a: 0 ]"
		// No error on "foo: NewType.new()"
		DO_EVAL("foo <- NewType"); // -> true

		DO_EVAL("foo.class"); //-> "NewType"
		DO_EVAL("foo.__type = NewType"); // -> true

		DO_EVAL("foo.a"); // -> 0
		DO_EVAL("foo.a: 3"); // -> 3
		DO_EVAL("foo.a - NewType.a"); // -> 3
		DO_EVAL("foo.__type.a != foo.a"); // -> true
										  // require dust_error on "foo._max: 3"

										  // No error on "baz: foo.copy()"
		DO_EVAL("baz.a:+ 2"); // -> 5
		DO_EVAL("baz.a != foo.a"); // -> true

								   // No error on "def NewType.new(self, a)\n\tself.a: a or 0\n"
		DO_EVAL("bar: NewType(5) <- NewType"); // -> true
		DO_EVAL("bar.a"); // -> 5
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
		DO_EVAL("3 + (a: 4)"); // -> 7
		DO_EVAL("a = 4"); // -> true

		// require pegtl::parse_error on "3 + a: 3"

		DO_EVAL("a, b: 3, (c: 4)"); // -> 4
		DO_EVAL("a = 3 and c = 4"); // -> true

		DO_EVAL("3 +-3"); // -> 0

		run(e, "a: 2");
		DO_EVAL("(a: 3) + 3 * a"); // -> 12
		DO_EVAL("a = 3"); // -> true
	}

	SECTION("Ternary Statement") {
		DO_EVAL("true and false or true"); // -> true
		DO_EVAL("!c and 4 or 5"); // -> 4

		// This is testing boolean shortcircuitting
		// require pegtl::parse_error on "false and a: 5"
		DO_EVAL("false and (a: 5)"); // -> false
		DO_EVAL("a = true"); // -> true

		DO_EVAL("a: b or 5"); // -> 3
		DO_EVAL("a = 3"); // -> true

		DO_EVAL("b: c and 4 or (c: 5)"); // -> 5
		DO_EVAL("b = 5 and c = b"); // -> true

		DO_EVAL("b: c and 4 or (c: 6)"); // -> 4
		DO_EVAL("b = 4 and c = 5"); // -> true
	}
	
	// TODO: Add
}