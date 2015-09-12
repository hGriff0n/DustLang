#pragma once

#include "Actions.h"
#include "..\Exceptions\exceptions.h"

#include <iostream>
#include <iomanip>

// TODO Improvements:
	// Add ability to assign "test arenas"
	// Improve formatting (Especially for strings)
	// Improve API

namespace dust {
	namespace test {

		template <class Stream>
		class Tester {
			private:
				EvalState& e;
				parse::AST tree;
				Stream& s;
				std::function<void(EvalState&)> reset;

				static const int TESTING_WEIGHT = 30;

				Stream& displayTestHeader(const std::string& code) {
					s << "[|] Running Test " << std::setw(5) << ++num_tests;

					if (code.size() > TESTING_WEIGHT - 8)
						return s << "input=\"" + code + "\"\n\t    Testing ";
					else
						return s << std::setw(TESTING_WEIGHT) << ("input=\"" + code + "\"") << " Testing ";
				}

				void evaluate(const std::string& code) {
					pegtl::parse<grammar, action>(code, code, tree);
					tree.pop()->eval(e);
				}
				Stream& printMsg(bool pass) {
					num_pass += pass;
					return s << "[" << (pass ? "O" : "X") << "] ";
				}
				void exitTest() {
					s << std::endl;
					reset(e);
				}

			public:
				int num_tests = 0, num_pass = 0;

				Tester(EvalState& _e, Stream& _s) : e{ _e }, s{ _s }, reset{ [](EvalState& e) { e.clear(); } } {
					s << std::setiosflags(std::ios::left);
				}

				void initSuite(const std::string& name, const std::string& desc, const std::function<void(EvalState&)>& init) {

				}
				void endSuite() {

				}

				// After executing the given code, the stack has the given number of elements
				void require_size(const std::string& code, size_t siz) {
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
				void require_type(const std::string& code, const std::string& typ) {
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
				void require_error(const std::string& code) {
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
				void require_true(const std::string& code) {
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
					displayTestHeader(code) << "for exception of type Exception during evaluation\n";

					try {
						evaluate(code);

						printMsg(false) << " Failed Test " << std::setw(5) << num_tests << "\"" << code << "\" did not throw an exception\n";

					} catch (Exception& e) {
						printMsg(true) << " Passed Test " << std::setw(5) << num_tests << "\"" << code << "\" threw the expected exception\n";

					} catch (...) {
						printMsg(false) << " Failed Test " << std::setw(5) << num_tests << "\"" << code << "\" threw an unexpected exception\n";
					}

					exitTest();
				}
		};

		void run_tests(EvalState&);
	}
}