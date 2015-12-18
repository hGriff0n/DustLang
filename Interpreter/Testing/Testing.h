#pragma once

#include "Actions.h"

#include "..\Exceptions\dust.h"
#include "..\Exceptions\logic.h"
#include "..\Exceptions\runtime.h"
#include "..\Exceptions\parsing.h"

#include <iomanip>
#include <vector>
#include "Console.h"

// TODO Improvements
	// Improve API
	// Change order of review printing (Currently prints sub-tests before parent tests, confusing to read)

namespace dust {

	// Error name strings for use in Testing error printing
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
		template<> const std::string name<stack_state_error>::is = "stack_state_error";
		template<> const std::string name<stack_type_error>::is = "stack_type_error";
		template<> const std::string name<storage_access_error>::is = "storage_access_error";
		template<> const std::string name<null_exception>::is = "null_exception";
		template<> const std::string name<conversion_error>::is = "conversion_error";

		template<> const std::string name<pegtl::parse_error>::is = "pegtl_error";
	}

	namespace test {

		/*
		 * Class that actually performs testing evaluation, etc.
		 */
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

				Stream& _displayTestHeader(const std::string& code) {
					s << buffer << "[|] Running Test " << std::setw(5) << num_tests;

					if (code.size() > TESTING_WEIGHT - 8)
						return s << "input=\"" + parse::escape(code) + "\"\n" << buffer << "\t    Testing ";
					else
						return s << std::setw(TESTING_WEIGHT) << ("input=\"" + code + "\"") << " Testing ";
				}

				// Construct and evaluate the AST for the given code segment
				void evaluate(const std::string& code) {
					parse::ScopeTracker scp{};
					pegtl::parse<grammar, action>(parse::trim(code), code, tree, scp);
					tree.pop()->eval(e);
				}

				// Print the pass/fail message and update num_pass
				Stream& printMsg(bool pass) {
					num_pass += pass;
					s << (pass ? console::green : console::red) << buffer << "[" << (pass ? "O" : "X") << "] ";
					return s;
				}

				Stream& _printMsg(bool pass) {
					s << (pass ? console::green : console::red) << buffer << "[" << (pass ? "O" : "X") << "] ";
					return s;
				}

				// Clean up internal state
				void exitTest() {
					s << std::endl;
					tree.clear();
					reset(e);
				}

				void _exitTest(bool success) {
					if (print_all) s << std::endl;

					num_pass += success;
					++num_tests;
					
					tree.clear();
					reset(e);
				}

			protected:
				std::string buffer;
				EvalState& e;
				Stream& s;
				int num_tests = 0, num_pass = 0;
				bool print_all;

			public:
				Tester(EvalState& _e, Stream& _s, const std::string& buf, bool print_all) : e{ _e }, s{ _s }, buffer{ buf }, reset{ DEFAULT_RESET }, print_all{ print_all } {
					s << std::setiosflags(std::ios::left);
				}

				// Execute the given code
				virtual void eval(const std::string& code) {
					evaluate(code);
				}

				// After executing the given code, the stack has the given number of elements
				virtual void requireSize(const std::string& code, size_t siz) {
					bool success = false;

					try {
						evaluate(code);

						if ((success = e.size() == siz)) {
							if (print_all) {
								_displayTestHeader(code) << "for stack size of " << siz << "\n";
								_printMsg(true) << " Passed Test " << std::setw(5) << num_tests << "Stack had size " << siz << " after execution\n";
							}
						} else {
							_displayTestHeader(code) << "for stack size of " << siz << "\n";
							_printMsg(false) << " Failed Test " << std::setw(5) << num_tests << "Stack did not have size " << siz << " after execution\n";
						}

					} catch (std::exception& e) {
						_displayTestHeader(code) << "for stack size of " << siz << "\n";
						_printMsg(false) << "Exception: \"" << e.what() << "\"\n";
					}

					_exitTest(success);
				}

				// After executing the given code, the top item on the stack has the given type
				virtual void requireType(const std::string& code, const std::string& typ) {
					bool success = false;

					try {
						evaluate(code);

						auto name = e.ts.getName(e.at().type_id);
						success = (name == typ);

						if (!success || print_all) {
							_displayTestHeader(code) << "for result of type " << typ << "\n";
							_printMsg(success) << (success ? " Passed Test " : " Failed Test") << std::setw(5) << num_tests << "Result of ";

							e.stream(s) << " had type " << name << "\n";
						}

					} catch (std::exception& e) {
						_displayTestHeader(code) << "for result of type " << typ << "\n";
						_printMsg(false) << "Exception: \"" << e.what() << "\"\n";
					}

					_exitTest(success);
				}

				// While executing the given code, an exception is thrown
				virtual void requireError(const std::string& code) {
					bool success = false;

					try {
						evaluate(code);

						_displayTestHeader(code) << "for exception during evaluation\n";
						_printMsg(false) << " Failed Test " << std::setw(5) << num_tests << "\"" << code << "\" did not throw an exception\n";

					} catch (...) {
						success = true;

						if (print_all) {
							_displayTestHeader(code) << "for exception during evaluation\n";
							_printMsg(true) << " Passed Test " << std::setw(5) << num_tests << "\"" << code << "\" threw an exception\n";
						}
					}

					_exitTest(success);
				}

				virtual void requireNoError(const std::string& code) {
					bool success = false;
					try {
						evaluate(code);

						success = true;
						if (print_all) {
							_displayTestHeader(code) << "for no exceptions during evaluation\n";
							_printMsg(true) << " Passed Test " << std::setw(5) << num_tests << "\"" << code << "\" did not throw an exception\n";
						}
					} catch (std::exception& err) {
						_displayTestHeader(code) << "for no exceptions during evaluation\n";
						_printMsg(false) << " Failed Test " << std::setw(5) << num_tests << "\"" << code << "\" threw " << err.what() << "\n";
					}

					_exitTest(success);
				}

				// While executing the given code, an exception, of type 'Exception', is thrown
				template <typename Exception>
				void requireException(const std::string& code) {
					bool success = false;

					try {
						evaluate(code);

						_displayTestHeader(code) << "for exception of type " << error::name<Exception>::is << "\n";
						_printMsg(false) << " Failed Test " << std::setw(5) << num_tests << "\"" << code << "\" did not throw an exception\n";

					} catch (Exception& e) {
						success = true;

						if (print_all) {
							_displayTestHeader(code) << "for exception of type " << error::name<Exception>::is << "\n";
							_printMsg(true) << " Passed Test " << std::setw(5) << num_tests << "Caught: " << e.what() << "\n";
						}

					} catch (std::exception& e) {
						_displayTestHeader(code) << "for exception of type " << error::name<Exception>::is << "\n";
						_printMsg(false) << " Failed Test " << std::setw(5) << num_tests << "Caught exception of type " << typeid(e).name() << "\n";
					}

					_exitTest(success);
				}

				// After executing the given code, the top value on the stack is the given value
				template <typename T>
				void requireEval(const std::string& code, const T& val) {
					bool success = false;

					try {
						evaluate(code);

						e.copy();
						success = (e.pop<T>() == val);

						if (success) {
							if (print_all) {
								_displayTestHeader(code) << "for result of " << val << "\n";
								_printMsg(true) << " Passed Test " << std::setw(5) << num_tests << "Expression evaluated to " << e.pop<T>() << "\n";
							}

						} else {
							_displayTestHeader(code) << "for result of " << val << "\n";
							_printMsg(false) << " Failed Test " << std::setw(5) << num_tests << "Result of " << e.pop<std::string>() << " did not match the expected value of " << val << "\n";
						}

					}  catch (std::exception& e) {
						_displayTestHeader(code) << "for result of " << val << "\n";
						_printMsg(false) << "Exception: \"" << e.what() << "\"\n";
					}

					_exitTest(success);
				}
				void requireEval(const std::string& code, const char* result) {
					return requireEval<std::string>(code, result);
				}

		};

		/*
		 * Class for organizing a multi-tiered testing system, with sub-tests
		 * Create this class in order to run tests (or call makeTester)
		 */
		template <class Stream>
		class TestOrganizer : public Tester<Stream> {
			static std::vector<std::pair<std::string, bool>> default_reviews;

			private:
				TestOrganizer<Stream>* sub_test = nullptr;
				std::string curr_test;
				std::vector<std::pair<std::string, bool>>& reviews;

			public:			// These constructors will cause recursion ???
				TestOrganizer(EvalState& _e, Stream& _s, bool print_all) : TestOrganizer{ _e, _s, print_all, "", default_reviews } {}
				TestOrganizer(EvalState& _e, Stream& _s, bool print_all, const std::string& buf) : TestOrganizer{ _e, _s, print_all, buf, default_reviews } {}
				TestOrganizer(EvalState& _e, Stream& _s, bool print_all, const std::string& buf, std::vector<std::pair<std::string, bool>>& _rws) : reviews{ _rws }, Tester{ _e, _s, buf, print_all } {}

				// Start a new sub_test
				void initSubTest(const std::string& msg) {
					if (sub_test) return sub_test->initSubTest(msg);

					s << buffer << "<:: " << (curr_test = msg) << " Testing ::>\n";
					sub_test = new TestOrganizer<Stream>{ e, s, print_all, buffer + " " };
				}

				// Close the sub test
				void closeSubTest() {
					if (!sub_test) return;

					// If there is a currently running sub_test, close that test
					if (sub_test->sub_test) return sub_test->closeSubTest();

					int np, nt;

					// Update parent num_test/num_pass data
					num_tests += (nt = sub_test->num_tests);
					num_pass += (np = sub_test->num_pass);

					// Create the review message for the sub test and add to the stack
					reviews.push_back(makeReview(buffer + " ", curr_test, np, nt));
					if (np != nt || print_all)
						s << reviews.back().first << "\n";

					delete sub_test;
					sub_test = nullptr;
				}

				virtual void requireSize(const std::string& code, size_t siz) {
					return sub_test ? sub_test->requireSize(code, siz) : Tester<Stream>::requireSize(code, siz);
				}

				// After executing the given code, the top item on the stack has the given type
				virtual void requireType(const std::string& code, const std::string& typ) {
					return sub_test ? sub_test->requireType(code, typ) : Tester<Stream>::requireType(code, typ);
				}

				// While executing the given code, an exception is thrown
				virtual void requireError(const std::string& code) {
					return sub_test ? sub_test->requireError(code) : Tester<Stream>::requireError(code);
				}

				// While executing the given code, no exceptions are thrown
				virtual void requireNoError(const std::string& code) {
					return sub_test ? sub_test->requireNoError(code) : Tester<Stream>::requireNoError(code);
				}

				// Executing the given code leaves a true value on top of the stack
				virtual void requireTrue(const std::string& code) {
					requireEval(code, true);
				}

				// After executing the given code, the top value on the stack is the given value
				template <typename T>
				void requireEval(const std::string& code, const T& val) {
					return sub_test ? sub_test->requireEval(code, val) : Tester<Stream>::requireEval(code, val);
				}

				// While executing the given code, an exception, of type 'Exception', is thrown
				template <typename Exception>
				void requireException(const std::string& code) {
					return sub_test ? sub_test->requireException<Exception>(code) : Tester<Stream>::requireException<Exception>(code);
				}

				// Print the review off all previously run sub-tests
				template <class OStream>
				void printReview(OStream& s) {
					if (!print_all) s << "\n";

					std::pair<std::string, bool> global = makeReview("", "Global Review", num_pass, num_tests);
					s << global.first;

					// Print all sub-tests (with requisite buffering)
					for (auto review : reviews)
						s << (review.second ? console::color{ 2 } : console::red) << review.first;

					s << global.first;
				}

				void printReview() {
					printReview(s);
				}
		};

		// Helper method for creating a Testing Environment
		template <class Stream>
		auto makeTester(EvalState& e, Stream& s, bool print_all) {
			return TestOrganizer<Stream>{ e, s, print_all };
		}

		// Helper method for creating a review message
		std::pair<std::string, bool> makeReview(const std::string&, const std::string&, int, int);

		// Run dust development tests
		void runTests(EvalState& e, bool print_all = true);
		//void run_regression_tests(EvalState& e, bool print_all = true);
		//void run_development_tests(EvalState& e, bool print_all = true);

		template <class Stream> std::vector<std::pair<std::string, bool>> TestOrganizer<Stream>::default_reviews = std::vector<std::pair<std::string, bool>>{};
		template <class Stream> const std::function<void(EvalState&)> Tester<Stream>::DEFAULT_RESET = [](EvalState& e) { e.clear(); };

	}
}