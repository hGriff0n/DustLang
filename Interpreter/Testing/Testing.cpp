#include "Testing.h"

#include "..\Exceptions\dust.h"
#include "..\Exceptions\logic.h"
#include "..\Exceptions\parsing.h"
#include "..\Exceptions\runtime.h"

#include <iostream>
#include <iomanip>

namespace dust {
	namespace test {

		Tester::Tester(EvalState& _e) : e{ _e }, reset{ [](EvalState& e) { e.clear(); } } {
			std::cout << std::setiosflags(std::ios::left);
		}

		void Tester::initSuite(const std::string& name, const std::string& desc, const std::function<void(EvalState&)>& init) {

		}

		void Tester::endSuite() {

		}

		void Tester::evaluate(const std::string& code) {
			pegtl::parse<grammar, action>(code, code, tree);
			tree.pop()->eval(e);
		}

		decltype(std::cout)& Tester::printMsg(bool pass) {
			return std::cout << "[" << (pass ? "O" : "X") << "] ";
		}

		void Tester::exitTest() {
			std::cout << std::endl;
			reset(e);
		}

		void Tester::require(const std::string& code, const std::string& test) {
			std::cout << "[|] Running Test " << std::setw(5) << ++num_tests << "input=\"" << code << "\"\n";

			try {
				evaluate(code);
				evaluate(test);

			} catch (pegtl::parse_error& e) {
				printMsg(false) << "Exception: \"" << e.what() << "\"\n";

				return exitTest();
			} catch (error::base& e) {
				printMsg(false) << "Exception: \"" << e.what() << "\"\n";

				return exitTest();
			}

			//e.copy();
			if (e.pop<bool>())
				printMsg(true) << " Passed Test " << std::setw(5) << num_tests << "\"" << test << "\" evaluated to true\n";
			else
				printMsg(false) << " Failed Test " << std::setw(5) << num_tests << "\"" << test << "\" evaluated to false\n";

			exitTest();
		}

		void Tester::require_size(const std::string& code, size_t siz) {
			std::cout << "[|] Running Test " << std::setw(5) << ++num_tests << "input=\"" << code << "\"\n";

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

		void Tester::require_type(const std::string& code, const std::string& typ) {
			std::cout << "[|] Running Test " << std::setw(5) << ++num_tests << "input=\"" << code << "\"\n";

			try {
				evaluate(code);

			} catch (pegtl::parse_error& e) {
				printMsg(false) << "Exception: \"" << e.what() << "\"\n";

				return exitTest();
			} catch (error::base& e) {
				printMsg(false) << "Exception: \"" << e.what() << "\"\n";

				return exitTest();
			}

			if (e.ts.getName(e.pop().type_id) == typ)
				printMsg(true) << " Passed Test " << std::setw(5) << num_tests << "Top value on the stack had type " << typ << "\n";
			else
				printMsg(false) << " Failed Test " << std::setw(5) << num_tests << "Top value on the stack did not have type " << typ << "\n";

			exitTest();
		}

		void Tester::require_error(const std::string& code) {
			std::cout << "[|] Running Test " << std::setw(5) << ++num_tests << "input=\"" << code << "\"\n";

			try {
				evaluate(code);

				printMsg(false) << " Failed Test " << std::setw(5) << num_tests << "\"" << code << "\" did not throw an exception\n";

			} catch (...) {
				printMsg(true) << " Passed Test " << std::setw(5) << num_tests << "\"" << code << "\" threw an exception\n";
			}

			exitTest();
		}
	
		void Tester::require_eval(const std::string& code, const char* result) {
			return require_eval<std::string>(code, result);
		}

	}
}