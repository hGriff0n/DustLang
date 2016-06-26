#include "Testing.h"

#define CATCH_CONFIG_RUNNER
#include "../libs/catch.hpp"

using namespace std;
using namespace dust;
using namespace dust::test;

int dust::test::runTests(int sel) {
	int res;

	switch (sel) {
		case 1:		// Start debugging when an exception gets thrown
			return Catch::Session().run(9, new char*[9]{ "DustTests", "--order", "decl", "-r", "junit", "-e", "-b", "--use-colour", "yes" });
		case 2:		// Output to a junit html doc
			res = Catch::Session().run(7, new char*[7]{ "DustTests", "--order", "decl", "-r", "junit", "-o", "DustTests.xml" });
			system("junit-viewer --results=DustTests.xml --save=DustTest.html && del DustTests.xml");
			return res;
		case 3:		// Display all tests regardless of result
			return Catch::Session().run(6, new char*[6]{ "DustTests", "--order", "decl", "-s", "--use-colour", "yes" });
		case 4:		// Run tests on backing structures
			return Catch::Session().run(6, new char*[6]{ "DustTests", "--order", "decl", "--use-colour", "yes", "back" });
		case 5:		// Run no tests
			return -1;
		default:	// Run default testing procedure
			return Catch::Session().run(5, new char*[5]{ "DustTests", "--order", "decl", "--use-colour", "yes" });
	}
}

// Rename to NO_THROW
#define NO_THROW(expr) e.clear(); REQUIRE_NOTHROW({ run(e, expr); })
#define MUST_THROW(exception_type, expr) REQUIRE_THROWS_AS({ run(e, expr); }, exception_type)
#define MAY_THROW(expr) CHECK_THROWS({ run(e, expr); })

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

// Only run when `back` tests are being performed
	// Actually split backing testing into several test cases (just be sure to have the same tag) 
TEST_CASE("Backing Structure Testing", "[.back]") {

}

TEST_CASE("Literal Parsing") {
	EvalState e;
	initState(e);
	
	// Test that simple literal values are recognized correctly
	SECTION("Simple Literals") {
		NO_THROW("3");
		REQUIRE(typeof(e) == "Int");
		REQUIRE((int)e == 3);

		NO_THROW("3.3");
		REQUIRE(typeof(e) == "Float");
		REQUIRE((double)e == 3.3);

		NO_THROW("\"Hello\"");
		REQUIRE(typeof(e) == "String");
		REQUIRE((string)e == "Hello");

		NO_THROW("true");
		REQUIRE(typeof(e) == "Bool");
		REQUIRE((bool)e);

		NO_THROW("false");
		REQUIRE(typeof(e) == "Bool");
		REQUIRE_FALSE((bool)e);

		NO_THROW("nil");
		REQUIRE(typeof(e) == "Nil");
	}

	// Test that simple table literals are recognized correctly
	SECTION("Tables") {
		NO_THROW("[]");
		REQUIRE(typeof(e) == "Table");

		NO_THROW("[ 1 2 ]");
		REQUIRE(typeof(e) == "Table");

		NO_THROW("[ 1, 2 ]");
		REQUIRE(typeof(e) == "Table");		// Error: Nil

		NO_THROW("[ a: 1 2 ]");
		REQUIRE(typeof(e) == "Table");
	}

	// Test that lambda functions are recognized correctly (TODO: In development)
	SECTION("Lambdas", "[!mayfail]") {
		NO_THROW("\\x -> x");
		REQUIRE(typeof(e) == "Function");

		// TODO: Add more lambda tests
	}
}

// Note: Move ternary tests here if I use them in any intervening tests
TEST_CASE("Operator Evaluation") {
	EvalState e;
	initState(e);

	// Test that unary operators evaluate correctly where defined
	SECTION("Unary Operators") {
		NO_THROW("!true");
		REQUIRE_FALSE((bool)e);

		NO_THROW("!false");
		REQUIRE((bool)e);

		NO_THROW("-3");
		REQUIRE((int)e == -3);

		NO_THROW("-3.3");
		REQUIRE((double)e == -3.3);
	}

	// Test that binary operators evaluate correctly where defined on the same type
	SECTION("Binary Operators") {
		NO_THROW("3 ^ 2");
		REQUIRE((int)e == 9);

		NO_THROW("3.3 ^ 2");
		REQUIRE((double)e == Approx(10.89));

		NO_THROW("3 * 2");
		REQUIRE((int)e == 6);

		NO_THROW("5 / 2");
		REQUIRE((double)e == 2.5);

		NO_THROW("\"Hello,\" + \" World!\"");
		REQUIRE((string)e == "Hello, World!");

		NO_THROW("3 - 4");
		REQUIRE((int)e == -1);

		NO_THROW("7 % 4");
		REQUIRE((int)e == 3);
	}

	// Test that boolean operators evalutate correctly where defined
	SECTION("Boolean Operators") {
		NO_THROW("\"a\" = \"b\"");
		REQUIRE_FALSE((bool)e);

		NO_THROW("3 != 4");
		REQUIRE((bool)e);

		NO_THROW("3.3 < 4.4");
		REQUIRE((bool)e);

		NO_THROW("5 > 3");
		REQUIRE((bool)e);

		NO_THROW("4 <= 4");
		REQUIRE((bool)e);

		NO_THROW("5.3 >= 5.4");
		REQUIRE_FALSE((bool)e);

		NO_THROW("true and false");
		REQUIRE_FALSE((bool)e);

		NO_THROW("true and true");
		REQUIRE((bool)e);

		NO_THROW("true or false");
		REQUIRE((bool)e);

		NO_THROW("false or false");
		REQUIRE_FALSE((bool)e);
	}

	// Test that operators evaluate correctly with operands of different types
	SECTION("Operator Resolution") {
		NO_THROW("3 * 2.5");
		REQUIRE(typeof(e) == "Float");
		REQUIRE((double)e == 7.5);

		NO_THROW("\"The Number \" + 3");
		REQUIRE(typeof(e) == "String");
		REQUIRE((string)e == "The Number 3");
	}

	// TODO: Later
	SECTION("Operator Precedence") {

	}
}

TEST_CASE("Variable Assignment") {
	EvalState e;
	initState(e);

	// Test that basic variable assignment evaluates as expected
	SECTION("Assignment Basics") {
		NO_THROW("a: 3");
		REQUIRE((int)e == 3);

		NO_THROW("a");
		REQUIRE((int)e == 3);

		NO_THROW("b: a");
		REQUIRE((int)e == 3);

		NO_THROW("(a: 2) + b");
		REQUIRE((int)e == 5);

		NO_THROW("a, b: b, a");
		REQUIRE((int)e == 2);

		NO_THROW("a");
		REQUIRE((int)e == 3);
	}

	// Test that compound assignment works correctly in its intricacies
	SECTION("Compound Assignment") {
		run(e, "a: 3");			// temporary setup ??
		run(e, "b: 0");

		NO_THROW("a, b:+ 2, 2");
		REQUIRE((int)e == 2);
		NO_THROW("a = 5 and b = 2");
		REQUIRE((bool)e);

		MUST_THROW(error::dispatch_error, "a, b:* 2");
		NO_THROW("a = 10");
		REQUIRE((bool)e);

		MUST_THROW(error::dispatch_error, "a, b:* nil, 2");
		NO_THROW("b = 2");
		REQUIRE((bool)e);

		run(e, "a: 15");

		NO_THROW("a:= b * (b: 3) + b ^ 2");
		REQUIRE((bool)e);
		NO_THROW("a and b = 3");
		REQUIRE((bool)e);
	}

	// Test that boolean operators perform shortcircuitting
	SECTION("Boolean Shortcircuiting") {
		run(e, "a: true");

		NO_THROW("false and (a: 5)");
		REQUIRE_FALSE((bool)e);

		NO_THROW("a");
		REQUIRE((bool)e);

		NO_THROW("true or (a: 3)");
		REQUIRE((bool)e);

		NO_THROW("a");
		REQUIRE((bool)e);
	}
}

// Consider rearranging some of these test cases
TEST_CASE("Tables") {
	EvalState e;
	initState(e);

	// Check that simple table indexing works correctly
	SECTION("Indexing") {
		run(e, "a, b: [ 1 ], 1");
		NO_THROW("a.1");
		REQUIRE((int)e == 1);
		NO_THROW("a.1 = a[b]");
		REQUIRE((bool)e);

		run(e, "a.a: [ a: 3 ]");
		NO_THROW("a"); // -> "[1, a: [ a: 3 ] ]"
		REQUIRE((string)e == "[ 1, a: [ a: 3 ] ]");
		NO_THROW("a.a"); // -> "[ a: 3 ]"
		REQUIRE((string)e == "[ a: 3 ]");
		NO_THROW("a.a.b: 2"); // -> 2
		REQUIRE((int)e == 2);
		NO_THROW("a.a"); // -> "[ a: 3, b: 2 ]"
		REQUIRE((string)e == "[ a: 3, b: 2 ]");
	}

	// Check that the default table operators work correctly
	SECTION("Operators") {
		run(e, "a: [ 1 2 3 2 5 5 4 ]");
		run(e, "b: [ 1 3 ]");
		run(e, "c: 5");

		NO_THROW("(a ^ b) = b");
		REQUIRE((bool)e);
		NO_THROW("a ^ c"); // -> "[ 5, 5 ]"
		REQUIRE((string)e == "[ 5, 5 ]");
		NO_THROW("b + c"); // -> "[ 1, 3, 5 ]"
		REQUIRE((string)e == "[ 1, 3, 5 ]");
		NO_THROW("a - b"); // -> "[ 2, 2, 5, 5, 4 ]"
		REQUIRE((string)e == "[ 2, 2, 5, 5, 4 ]");
		NO_THROW("b * c"); // -> "[ 1, 3, 5 ]"
		REQUIRE((string)e == "[ 1, 3, 5 ]");
	}

	// Check that advanced table constructions evaluate correctly
	SECTION("Advanced") {
		run(e, "a, b: [ 1 2 3 2 5 5 4 ], [ 1 3 ]");

		NO_THROW("a[b[2]]");
		REQUIRE((int)e == 3);
		NO_THROW("a[b[2] * 2]");
		REQUIRE((int)e == 5);
	}

	// Experimental to see whether the given construction fails
	SECTION("Experimental", "[!mayfail]") {
		NO_THROW("[2].1");
		REQUIRE((int)e == 2);
	}
}

TEST_CASE("Advanced Parsing") {
	EvalState e;
	initState(e);

	// Ensure that comments dont affect evaluation
	SECTION("Comments") {
		NO_THROW("## This is a comment");
		REQUIRE(typeof(e) == "Nil");

		NO_THROW("3## And a comment");
		REQUIRE((int)e == 3);

		NO_THROW("");
		REQUIRE(typeof(e) == "Nil");
	}

	// Test that the parser can handle input from multiple lines
	SECTION("Multiline Input") {
		run(e, "a, b: 4, 3");			// temporary setup ???

		NO_THROW("a\n+ b");
		REQUIRE((int)e == 7);

		NO_THROW("3 - \n 3");
		REQUIRE((int)e == 0);

		NO_THROW("3 + 3\n  \n4 + 4");
		REQUIRE((int)e == 8);

		NO_THROW("## Hello\n3");
		REQUIRE((int)e == 3);
	}

	// Test that scopes and scoping rules are handled correctly
	SECTION("Scopes") {
		NO_THROW("a: 2\n	a:5\n	a");
		REQUIRE((int)e == 5);
		NO_THROW("a = 2");
		REQUIRE((bool)e);
		NO_THROW("a: 3\n	a + 3");
		REQUIRE((int)e == 6);
		
		NO_THROW("a: 4\n"
			"	a: 3\n"
			"	b: a + .a");
		REQUIRE((int)e == 7);
		NO_THROW(".a");
		REQUIRE((int)e == 4);

		NO_THROW("a: 2 ## Assign 2 to a\n"
			"	b: .a + 3 ## Assign a + 3 to b\n"
			"	b + a");
		REQUIRE((int)e == 7);

		NO_THROW("a: 2\n"
			"	b: 3 + .a\n"
			"## Assign 3 + a to b\n"
			"	b + a");
		REQUIRE((int)e == 7);

		NO_THROW("a: 2\n"
			"	b: 3 + .a\n"
			"## Assign 3 + a to b"
			"	b + a");
		REQUIRE((int)e == 5);

		NO_THROW("af: 3\n"
			"		5\n"
			"	af");
		REQUIRE((int)e == 3);
	}
}

// Test that I can reliably throw exceptions  from dust code
TEST_CASE("Exceptions") {
	EvalState e;
	initState(e);

	// Test that exceptions are thrown where they should be
	SECTION("Exceptions") {
		MUST_THROW(error::dispatch_error, "-\"3\"");
		MAY_THROW("!1");

		MUST_THROW(pegtl::parse_error, "false and a: 5");

		MUST_THROW(pegtl::parse_error, "a: <- Int");				// Error: No exception thrown
		MUST_THROW(pegtl::parse_error, "for i in [ 1 2 3 4 5 ] .sum: + i");
	}
}

TEST_CASE("Control Flow Constructions") {
	EvalState e;
	initState(e);

	// Test that while loops evaluate correctly
	SECTION("While Loop") {
		NO_THROW("i: 0\n"
			"while i < 5\n"
			"	.i:+ 1\n"
			"i");
		REQUIRE((int)e == 5);

		NO_THROW("i: 0\n"
			"while i < 5 .i:+ 1\n");
		REQUIRE((int)e == 5);

		NO_THROW("i: 0\n"
			"while i < 5 do\n"
			"	.i:+ 1\n");
		REQUIRE((int)e == 5);

		NO_THROW("i: 5\n"
			"while i < 5 .i:+ 1\n"); // -> nil
		REQUIRE(typeof(e) == "Nil");
	}

	// Test that do-while (repeat) loops evaluate correctly
	SECTION("Do-While Loop") {
		NO_THROW("i: 0\n"
			"repeat i < 5\n"
			"	.i:+ 1\n"
			"i");
		REQUIRE((int)e == 5);

		NO_THROW("i: 0\n"
			"repeat i < 5 .i:+ 1");
		REQUIRE((int)e == 5);

		NO_THROW("i: 5\n"
			"repeat i < 5 .i:+ 1");
		REQUIRE((int)e == 6);
	}

	// Test that basic for loops work correctly
	SECTION("For Each Loop") {
		NO_THROW("sum: 0\n"
			"for i in [ 1 2 3 4 5 ]\n"
			"	.sum:+ i\n"
			"sum");
		REQUIRE((int)e == 15);

		NO_THROW("for i in [1]"); // -> nil
		REQUIRE(typeof(e) == "Nil");

		NO_THROW("msg: \"\"\n"
			"for w in [ \"Hello,\" \"World!\" \"I'm\" \"Margaret\" ]\n"
			"	.msg:+ w + \" \"\n"
			"msg");
		REQUIRE((string)e == "Hello, World! I'm Margaret ");

		run(e, "a: [ 1 2 b: [ 3 4 5 ] ]");
		NO_THROW("f: 1\n"
			"for i in a.b .f:* i\n");
		REQUIRE((int)e == 60);

		run(e, "a: [ 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 ]");
		NO_THROW("sum_a, sum_b: 0, 0\n"
			"for i in (a ^ [ 1 2 3 5 7 11 13 17 ])\n"
			"	.sum_a:+ i\n"
			"for i in [ 1 2 3 5 7 11 13 17 ]\n"
			"	.sum_b:+ i\n"
			"sum_a = sum_b");
		REQUIRE((bool)e);
	}

	// Test that if statements parse and evaluate correctly
	SECTION("If Statement") {
		run(e, "i: 5");

		NO_THROW("if i 3");
		REQUIRE((int)e == 3);
		NO_THROW("if i = 3 6");
		REQUIRE_FALSE((bool)e);
		NO_THROW("if i = 3 6\n"
			"else 7");
		REQUIRE((int)e == 7);

		NO_THROW("if i = 3 6\n"
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

	// Test that try-catch blocks handle exceptions correctly
	SECTION("Try-Catch Block") {
		NO_THROW("try\n"
			"	3 + true\n"
			"catch (e) 4\n");
		REQUIRE((int)e == 4);

		NO_THROW("try 3 + true\n"
			"catch (e) 4");
		REQUIRE((int)e == 4);

		NO_THROW("try 3 + 3\n"
			"catch (e) 4");
		REQUIRE((int)e == 6);

		NO_THROW("a: 3\n"
			"try\n"
			"	3 + true\n"
			"	.a: 2\n"
			"catch (e)\n"
			"a");
		REQUIRE((int)e == 3);

		NO_THROW("a: 3\n"
			"try\n"
			"	3 + 3\n"
			"	.a: 2\n"
			"catch (e)\n"
			"a");
		REQUIRE((int)e == 2);

		NO_THROW("try\n"
			"	.a: 3 + 3\n"
			"	3 + a\n"
			"catch(e)\n");
		REQUIRE((int)e == 9);

		//NO_THROW("(try 3 + true\n"
		//	"catch (e) e) <- String");		// ERROR: matching struct c_paren
		NO_THROW("try 3 + true\n"
			"catch(e) .a: e\n"
			"a <- String");
		REQUIRE((bool)e);
		
		MUST_THROW(error::missing_node_x, "catch (e) 4");
	}

	// TODO: Later
	SECTION("For Loop Generator") {

	}
}

// TODO: Later
TEST_CASE("Type System Basics") {
	EvalState e;
	initState(e);

	SECTION("Type Checking") {
		NO_THROW("3 <- Int");
		REQUIRE((bool)e);

		NO_THROW("3 + 0.3 <- Float");
		REQUIRE((bool)e);
	}

	SECTION("Meta-values") {

	}
}

TEST_CASE("Functions") {
	EvalState e;
	initState(e);

	e.push("abs");
	e.push([](EvalState& e) {
		auto x = (int)e;
		e.push(x > 0 ? x : -x);
		return 1;
	});
	e.set(EvalState::SCOPE);

	e.addMember(e.getTS().getType("Int"), "abs", [](EvalState& e) {
		e.enableObjectSyntax().get(EvalState::SELF);

		auto x = (int)e;
		e.push(x > 0 ? x : -x);
		return 1;
	});

	e.push("add");
	e.push([](EvalState& e) {
		e.push((int)e + (int)e);
		return 1;
	});
	e.set(EvalState::SCOPE);

	e.push("bound");
	e.push([](EvalState& e) {
		auto d = (double)e;
		e.push((int)std::floor(d));
		e.push((int)std::ceil(d));
		return 2;
	});
	e.set(EvalState::SCOPE);

	e.push("give5");
	e.push([](EvalState& e) {
		e.push(5);
		return 1;
	});
	e.set(EvalState::SCOPE);

	// Test that calling functions (free and OOP) works correctly
	SECTION("Calling Functions") {
		NO_THROW("abs(1)");
		REQUIRE((int)e == 1);
		NO_THROW("abs(-1)");
		REQUIRE((int)e == 1);

		NO_THROW("add(2, 3)");
		REQUIRE((int)e == 5);
		NO_THROW("add( 1 + 1,3)");
		REQUIRE((int)e == 5);

		NO_THROW("give5()");
		REQUIRE((int)e == 5);
		NO_THROW("give5(3)");
		REQUIRE(e.size() == 1);
		REQUIRE((int)e == 5);

		MUST_THROW(error::runtime_error, "add(2)");

		e.push("max");
		e.push([](EvalState& e) {
			Optional max{ e }, opt;							// Max may have a value, opt guaranteed to be nil

			while (opt.set(e)) {							// For each optional argument, try to beat max
				e.push(max);
				e.push(opt);
				e.callOp("_op>");							// Run opt > max

				if ((bool)e) max.set(opt);
			}

			e.push(max);
			return 1;
		});
		e.set(EvalState::SCOPE);

		NO_THROW("max(3, 5)");
		REQUIRE((int)e == 5);
		NO_THROW("max(3)");
		REQUIRE((int)e == 3);

		MUST_THROW(error::dispatch_error, "foo(3)");

		run(e, "a: -1");
		NO_THROW("a.abs()");
		REQUIRE((int)e == 1);
		NO_THROW("-1.abs()");
		REQUIRE((int)e == -1);
		NO_THROW("(-1).abs()");
		REQUIRE((int)e == 1);
		NO_THROW("(5 * -2).abs()");
		REQUIRE((int)e == 10);
		NO_THROW("Int.abs(a - 3)");
		REQUIRE((int)e == 4);

		MUST_THROW(error::dispatch_error, "\"Hello\".abs()");
	}

	// Test that functions can be passed around as value
	SECTION("Functions as First-Class Values") {
		run(e, "sba: abs");
		NO_THROW("sba <- Function");
		REQUIRE((bool)e);
		NO_THROW("sba(-3) = abs(3)");
		REQUIRE((bool)e);

		run(e, "a: [ abs: abs ]");
		NO_THROW("a.abs(3) = 3.abs()");
		REQUIRE((bool)e);

		run(e, "abs: 5");
		NO_THROW("abs <- Int");
		REQUIRE((bool)e);
		NO_THROW("sba <- Function");
		REQUIRE((bool)e);
	}

	// Test that functions can be created using the 'def' keyword
	SECTION("`def`-syntax") {
		NO_THROW("def min(x, y)\n"
			"	if x < y x\n"
			"	else y");
		NO_THROW("min <- Function");
		REQUIRE((bool)e);

		NO_THROW("min(3, 5)");
		REQUIRE((int)e == 3);
		NO_THROW("min(3, 5, 7)");
		REQUIRE(e.size() == 1);
		REQUIRE((int)e == 3);

		MUST_THROW(error::dispatch_error, "min(3)");

		NO_THROW("def Float.abs(self)\n"
			"	self < 0 and -self or self");
		NO_THROW("Float.abs <- Function");
		REQUIRE((bool)e);

		NO_THROW("Float.abs(-3.3)");
		REQUIRE((double)e == 3.3);
		NO_THROW("(-5.5).abs()");
		REQUIRE((double)e == 5.5);
		NO_THROW("5.5.abs()");
		REQUIRE((double)e == 5.5);

		NO_THROW("def Bool._op+(self, o)\n"
			"	self or o");
		NO_THROW("true + false");
		REQUIRE((bool)e);

		NO_THROW("def Int.diverge(a)\n"
			"	return a + 1, a - 1");
		NO_THROW("a, b: Int.diverge(3)");
		REQUIRE((int)e == 4);
		NO_THROW("a + b");
		REQUIRE((int)e == 6);

		MUST_THROW(error::dispatch_error, "3.diverge()");
	}

	// Test that functions can return multiple values
	SECTION("Multiple Return Values") {
		NO_THROW("a, b: bound(3.3)");
		REQUIRE((int)e == 4);
		NO_THROW("a + b");
		REQUIRE((int)e == 7);
		NO_THROW("add(bound(4.7))");
		REQUIRE((int)e == 9);
	}

	// Test that recursion works
	SECTION("Recursion", "[!mayfail]") {
		run(e, "def factorial(n)\n"
			"	if n = 0 1\n"
			"	else n * factorial(n - 1)");

		NO_THROW("factorial(0)");
		REQUIRE((int)e == 1);
		NO_THROW("factorial(3)");			// ERROR: Index stack of size 1 with (size_t)-1
		REQUIRE((int)e == 6);
	}
}

TEST_CASE("User-defined Types") {
	EvalState e;
	initState(e);

	// Ensure that custom types can be created
	SECTION("Definition Syntax") {
		NO_THROW("type NewType [ a: 0 ]");
		NO_THROW("def NewType.new(self, a)\n"
			"	self.a: a or 0");

		NO_THROW("type TestType [ b: 0 ]\n"
			"def TestType.new(self, b)\n"
			"	self.b: b and b * 2 or 3");
	}

	// TODO: Combine with above Section?
	SECTION("Using default type-methods") {
		run(e, "type NewType [ a: 0 ]");

		NO_THROW("foo: NewType.new()");
		NO_THROW("foo <- NewType");
		REQUIRE((bool)e);

		NO_THROW("foo.class");
		REQUIRE((string)e == "NewType");
		NO_THROW("foo.__type = NewType");
		REQUIRE((bool)e);

		NO_THROW("foo.a");
		REQUIRE((int)e == 0);
		NO_THROW("foo.a: 3");
		REQUIRE((int)e == 3);
		NO_THROW("foo.a - NewType.a");
		REQUIRE((int)e == 3);
		NO_THROW("foo.__type.a != foo.a");
		REQUIRE((bool)e);
		MUST_THROW(error::dust_error, "foo._max: 3");

		NO_THROW("baz: foo.copy()");
		NO_THROW("baz.a:+ 2");
		REQUIRE((int)e == 5);
		NO_THROW("baz.a != foo.a");
		REQUIRE((bool)e);
	}

	// Check whether custom type methods evaluate correctly
	SECTION("Using custom type-methods") {
		run(e, "type NewType [ a: 0 ]");
		run(e, "def NewType.new(self, a)\n"
			"	self.a: a or 0");

		NO_THROW("foo: NewType.new(5)");
		REQUIRE(typeof(e) == "NewType");

		NO_THROW("foo <- NewType");
		REQUIRE((bool)e);

		NO_THROW("foo.a");
		REQUIRE((int)e == 5);
	}

	// Ensure that static members work correctly
	SECTION("Types with Static Members") {
		run(e, "type NewType [ a: 0 ]");

		NO_THROW("NewType.count: 0");
		REQUIRE((int)e == 0);

		run(e, "def NewType.new(self, a)\n"
			"	NewType.count:+ 1\n"
			"	self.a: a or 0\n");

		NO_THROW("foo: NewType.new(3)");
		NO_THROW("NewType.count");
		REQUIRE((int)e == 1);

		MUST_THROW(error::dust_error, "foo.count: 5");
	}

	// Ensure that inheritance works with custom types
		// TODO: Add tests for inheritance with custom new/etc.
	SECTION("Inheritance") {
		run(e, "type NewType [ a: 0 ]");
		run(e, "def NewType.inc(self)\n"
			"	self.a:+ 1");

		NO_THROW("type Derived [ b: 1 ] <- NewType");
		NO_THROW("bar: Derived.new()");
		REQUIRE(typeof(e) == "Derived");

		NO_THROW("bar.a");
		REQUIRE((int)e == 0);		// Error: Can't convert to Int
		NO_THROW("bar.b");
		REQUIRE((int)e == 1);

		NO_THROW("bar.inc()");
		REQUIRE((int)e == 1);
		NO_THROW("bar.a");
		REQUIRE((int)e == 1);
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
		NO_THROW("3 + (a: 4)");
		REQUIRE((int)e == 7);
		NO_THROW("a = 4");
		REQUIRE((bool)e);

		MUST_THROW(pegtl::parse_error, "3 + a: 3");

		NO_THROW("a, b: 3, (c: 4)");
		REQUIRE((int)e == 4);
		NO_THROW("a = 3 and c = 4");
		REQUIRE((bool)e);

		NO_THROW("3 +-3");
		REQUIRE((int)e == 0);
	}

	SECTION("Ternary Statement") {
		run(e, "a, b: true, 3");

		NO_THROW("true and false or true");			// Odd, but dust's ternary is modeled after Lua's and this is the correct behavior
		REQUIRE((bool)e);
		NO_THROW("!c and 4 or 5");
		REQUIRE((int)e == 4);

		NO_THROW("a: b or 5");
		REQUIRE((int)e == 3);
		NO_THROW("a = 3");
		REQUIRE((bool)e);

		NO_THROW("b: c and 4 or (c: 5)");
		REQUIRE((int)e == 5);
		NO_THROW("b = 5 and c = b");
		REQUIRE((bool)e);

		NO_THROW("b: c and 4 or (c: 6)");
		REQUIRE((int)e == 4);
		NO_THROW("b = 4 and c = 5");
		REQUIRE((bool)e);
	}
	
	// TODO: Add
}
