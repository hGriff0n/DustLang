#pragma once

#include "Actions.h"
#include "..\Exceptions\exceptions.h"

#include <iostream>
#include <iomanip>

// TODO Improvements:
	// Move "passing" code into the try blocks
		// Reduces code complexity and "ugliness"
	// Don't rely on string comparisons to test equality
		// This will likely entail templates (move to .h)
		// This is necessary for comparing Floats
	// Add ability to query multiple assignments
	// Add ability to assign "test arenas"
	// Improve response message helpfulness
		// Give some indication as to where the execution failed
	// Possibly template the stream (either for the functions or for the class)
	// Improve API

namespace dust {
	namespace test {

		class Tester {
			private:
				EvalState& e;
				parse::AST tree;
				std::function<void(EvalState&)> reset;
				int num_tests = 0, num_pass = 0;

				void evaluate(const std::string&);
				decltype(std::cout)& printMsg(bool);
				void exitTest();

			public:
				Tester(EvalState&);

				void initSuite(const std::string&, const std::string&, const std::function<void(EvalState&)>&);
				void endSuite();

				//void require(const std::string&, const std::vector<const std::string&>&);

				void require(const std::string&, const std::string&);
				void require_size(const std::string&, size_t);
				void require_type(const std::string&, const std::string&);
				void require_error(const std::string&);

				template <typename T>
				void require_eval(const std::string& code, const T& val) {
					std::cout << "[|] Running Test " << std::setw(5) << ++num_tests << "input=\"" << code << "\"\n";

					try {
						evaluate(code);

						e.copy();
						if (e.pop<T>() == val)
							printMsg(true) << " Passed Test " << std::setw(5) << num_tests << e.pop<T>() << " was the top value on the stack\n";		// This e.pop<T> will never throw
						else
							printMsg(false) << " Failed Test " << std::setw(5) << num_tests << "Expected \"" << code << " = " << val << "\". Got \"" << code << " = " << e.pop<T>() << "\"\n";

					} catch (pegtl::parse_error& e) {
						printMsg(false) << "Exception: \"" << e.what() << "\"\n";
					} catch (error::base& e) {
						printMsg(false) << "Exception: \"" << e.what() << "\"\n";
					}

					exitTest();
				}
				void require_eval(const std::string& code, const char*);
		};

		void run_tests(EvalState&);
	}
}