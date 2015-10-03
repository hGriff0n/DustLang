#pragma once

#include "Actions.h"
#include "..\Exceptions\dust.h"
#include "..\Exceptions\logic.h"
#include "..\Exceptions\runtime.h"
#include "..\Exceptions\parsing.h"

#include <iostream>
#include <iomanip>

// TODO Improvements
	// Improve API
	// Surround require_eval output in quotes if the expected type is a string

namespace dust {
	namespace error {
		template <class Exception>
		struct name { static const std::string is; };

		template <class Exception> const std::string name<Exception>::is = "This is not an exception!";
		template<> const std::string name<base>::is = "base";

		template<> const std::string name<dust_error>::is = "dust_error";
		template<> const std::string name<dispatch_error>::is = "dispatch_error";
		template<> const std::string name<converter_not_found>::is = "converter_not_found";
		template<> const std::string name<illegal_operation>::is = "illegal_operation";
		template<> const std::string name<unimplemented_operation>::is = "unimplemented_operation";

		template<> const std::string name<logic_error>::is = "logic_error";
		template<> const std::string name<out_of_bounds>::is = "out_of_bounds";
		template<> const std::string name<illegal_template>::is = "illegal_template";
		template<> const std::string name<syntax_error>::is = "syntax_error";

		template<> const std::string name<parse_error>::is = "parse_error";
		template<> const std::string name<missing_nodes>::is = "missing_nodes";
		template<> const std::string name<missing_node_x>::is = "missing_node_x";
		template<> const std::string name<operands_error>::is = "operands_error";
		template<> const std::string name<invalid_ast_construction>::is = "invalid_ast_construction";

		template<> const std::string name<runtime_error>::is = "runtime_error";
		template<> const std::string name<bad_api_call>::is = "bad_api_call";
		template<> const std::string name<bad_node_eval>::is = "bad_node_eval";
		template<> const std::string name<incomplete_node>::is = "incomplete_node";
		template<> const std::string name<stack_state_error>::is = "stack_state_error";
		template<> const std::string name<stack_type_error>::is = "stack_type_error";
		template<> const std::string name<storage_access_error>::is = "storage_access_error";
		template<> const std::string name<null_exception>::is = "null_exception";
		template<> const std::string name<conversion_error>::is = "conversion_error";

		template<> const std::string name<pegtl::parse_error>::is = "pegtl_error";
	}

	namespace test {

		template <class Stream>
		class Tester {
			private:
				parse::AST tree;
				std::function<void(EvalState&)> reset;

				static const int TESTING_WEIGHT = 30;
				static const std::function<void(EvalState&)> DEFAULT_RESET;


				Stream& displayTestHeader(const std::string& code) {
					s << buffer << "[|] Running Test " << std::setw(5) << ++num_tests;

					if (code.size() > TESTING_WEIGHT - 8)
						return s << "input=\"" + parse::escape(code) + "\"\n" << buffer << "\t    Testing ";
					else
						return s << std::setw(TESTING_WEIGHT) << ("input=\"" + code + "\"") << " Testing ";
				}

				void evaluate(const std::string& code) {
					pegtl::parse<grammar, action>(code, code, tree, 0);
					tree.pop()->eval(e);
				}

				Stream& printMsg(bool pass) {
					num_pass += pass;
					return s << buffer << "[" << (pass ? "O" : "X") << "] ";
				}

				void exitTest() {
					s << std::endl;
					tree.clear();
					reset(e);
				}

			protected:
				std::string buffer;
				EvalState& e;
				Stream& s;

			public:
				int num_tests = 0, num_pass = 0;

				Tester(EvalState& _e, Stream& _s, const std::string& buf) : e{ _e }, s{ _s }, buffer{ buf }, reset { DEFAULT_RESET } {
					s << std::setiosflags(std::ios::left);
				}

				// Execute the given code
				virtual void eval(const std::string& code) {
					evaluate(code);
				}

				// After executing the given code, the stack has the given number of elements
				virtual void require_size(const std::string& code, size_t siz) {
					displayTestHeader(code) << "for stack size of " << siz << "\n";

					try {
						evaluate(code);

					} catch (pegtl::parse_error& e) {
						printMsg(false) << "Exception: \"" << e.what() << "\"\n";

						return exitTest();
					} catch (error::base& e) {
						printMsg(false) << "Exception: \"" << e.what() << "\"\n";

						return exitTest();
					}

					if (e.size() == siz)
						printMsg(true) << " Passed Test " << std::setw(5) << num_tests << "Stack had size " << siz << " after execution\n";
					else
						printMsg(false) << " Failed Test " << std::setw(5) << num_tests << "Stack did not have size " << siz << " after execution\n";

					exitTest();
				}

				// After executing the given code, the top item on the stack has the given type
				virtual void require_type(const std::string& code, const std::string& typ) {
					displayTestHeader(code) << "for result of type " << typ << "\n";

					try {
						evaluate(code);

					} catch (pegtl::parse_error& e) {
						printMsg(false) << "Exception: \"" << e.what() << "\"\n";

						return exitTest();
					} catch (error::base& e) {
						printMsg(false) << "Exception: \"" << e.what() << "\"\n";

						return exitTest();
					}

					auto name = e.ts.getName(e.at().type_id);
					(name == typ ? (printMsg(true) << " Passed Test ")
						: (printMsg(false) << " Failed Test "))
						<< std::setw(5) << num_tests
						<< "Result of " << e.pop<std::string>() << " had type " << name << "\n";

					exitTest();
				}

				// While executing the given code, an exception is thrown
				virtual void require_error(const std::string& code) {
					displayTestHeader(code) << "for exception during evaluation\n";

					try {
						evaluate(code);

						printMsg(false) << " Failed Test " << std::setw(5) << num_tests << "\"" << code << "\" did not throw an exception\n";

					} catch (...) {
						printMsg(true) << " Passed Test " << std::setw(5) << num_tests << "\"" << code << "\" threw an exception\n";
					}

					exitTest();
				}

				// Executing the given code leaves a true value on top of the stack
				virtual void require_true(const std::string& code) {
					displayTestHeader(code) << "for result of true\n";

					try {
						evaluate(code);

					} catch (pegtl::parse_error& e) {
						printMsg(false) << "Exception: \"" << e.what() << "\"\n";

						return exitTest();
					} catch (error::base& e) {
						printMsg(false) << "Exception: \"" << e.what() << "\"\n";

						return exitTest();
					}

					if (e.pop<bool>())
						printMsg(true) << " Passed Test " << std::setw(5) << num_tests << "\"" << code << "\" evaluated to true\n";
					else
						printMsg(false) << " Failed Test " << std::setw(5) << num_tests << "\"" << code << "\" evaluated to false\n";

					exitTest();
				}

				// After executing the given code, the top value on the stack is the given value
				template <typename T>
				void require_eval(const std::string& code, const T& val) {
					displayTestHeader(code) << "for result of " << val << "\n";

					try {
						evaluate(code);

						e.copy();
						if (e.pop<T>() == val)
							printMsg(true) << " Passed Test " << std::setw(5) << num_tests << "Expression evaluated to " << e.pop<T>() << "\n";		// This e.pop<T> will never throw
						else
							printMsg(false) << " Failed Test " << std::setw(5) << num_tests << "Result of " << e.pop<std::string>() << " did not match the expected value of " << val << "\n";

					} catch (pegtl::parse_error& e) {
						printMsg(false) << "Exception: \"" << e.what() << "\"\n";
					} catch (error::base& e) {
						printMsg(false) << "Exception: \"" << e.what() << "\"\n";
					}

					exitTest();
				}
				void require_eval(const std::string& code, const char* result) {
					return require_eval<std::string>(code, result);
				}

				// While executing the given code, an exception, of type 'Exception', is thrown
				template <typename Exception>
				void require_excep(const std::string& code) {
				displayTestHeader(code) << "for exception of type " << error::name<Exception>::is << "\n";

				try {
					evaluate(code);

					printMsg(false) << " Failed Test " << std::setw(5) << num_tests << "\"" << code << "\" did not throw an exception\n";

				} catch (Exception& e) {
					//printMsg(true) << " Passed Test " << std::setw(5) << num_tests << "\"" << code << "\" threw the expected exception\n";
					printMsg(true) << " Passed Test " << std::setw(5) << num_tests << "Caught: " << e.what() << "\n";

				} catch (...) {
					printMsg(false) << " Failed Test " << std::setw(5) << num_tests << "\"" << code << "\" threw an unexpected exception\n";
				}

				exitTest();
			}
		};

		template <class Stream>
		class TestOrganizer : public Tester<Stream> {
			private:
				TestOrganizer<Stream>* sub_test = nullptr;
				std::string curr_test;

			public:			// These constructors will cause recursion ???
				TestOrganizer(EvalState& _e, Stream& _s) : TestOrganizer(_e, _s, "") {}
				TestOrganizer(EvalState& _e, Stream& _s, const std::string& buf) : Tester{ _e, _s, buf } {}

				// Start a new sub_test
				void init_sub_test(const std::string& msg) {
					if (sub_test) return sub_test->init_sub_test(msg);

					s << buffer << "<:: " << (curr_test = msg) << " Testing ::>\n";
					sub_test = new TestOrganizer<Stream>{ e, s, buffer + " " };
				}

				// Close the sub test
				void close_sub_test() {
					if (!sub_test) return;
					if (sub_test->sub_test) return sub_test->close_sub_test();

					int np, nt;

					num_tests += nt = sub_test->num_tests;
					num_pass += np = sub_test->num_pass;

					s << buffer << " " << curr_test << " | Passed: " << np << " | Failed: " << (nt - np) << " | " 
								<< np << " / " << nt << " Tests (" << std::setprecision(4) << ((float)np / nt * 100) << "%)\n\n";

					delete sub_test;
					sub_test = nullptr;
				}

				virtual void require_size(const std::string& code, size_t siz) {
					return sub_test ? sub_test->require_size(code, siz) : Tester<Stream>::require_size(code, siz);
				}

				// After executing the given code, the top item on the stack has the given type
				virtual void require_type(const std::string& code, const std::string& typ) {
					return sub_test ? sub_test->require_type(code, typ) : Tester<Stream>::require_type(code, typ);
				}

				// While executing the given code, an exception is thrown
				virtual void require_error(const std::string& code) {
					return sub_test ? sub_test->require_error(code) : Tester<Stream>::require_error(code);
				}

				// Executing the given code leaves a true value on top of the stack
				virtual void require_true(const std::string& code) {
					return sub_test ? sub_test->require_true(code) : Tester<Stream>::require_true(code);
				}

				// After executing the given code, the top value on the stack is the given value
				template <typename T>
				void require_eval(const std::string& code, const T& val) {
					return sub_test ? sub_test->require_eval(code, val) : Tester<Stream>::require_eval(code, val);
				}

				// While executing the given code, an exception, of type 'Exception', is thrown
				template <typename Exception>
				void require_excep(const std::string& code) {
					return sub_test ? sub_test->require_excep<Exception>(code) : Tester<Stream>::require_excep<Exception>(code);
				}
		};

		void run_tests(EvalState&);

		template <class Stream> const std::function<void(EvalState&)> Tester<Stream>::DEFAULT_RESET = [](EvalState& e) { e.clear(); };

	}
}