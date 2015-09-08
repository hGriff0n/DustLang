#pragma once

#include "Actions.h"

// Not very helpful in response
	// It really only works to say "This failed" and "That succeeded
	// It gives no indication where the execution failed

// Relies on string comparisons to test equality
	// Especially bad in regards to Floats (I don't know what the precision is)

// Can't test multiple assignments really well

// Haven't settled on a way to effectively set a "reset" point


namespace dust {
	namespace test {

		class Tester {
			private:
				EvalState& e;
				parse::AST tree;
				std::function<void(EvalState&)> reset;
				int num_tests = 0, num_pass = 0;

				void evaluate(const std::string&);

			public:
				Tester(EvalState&);

				void initSuite(const std::string&, const std::string&, const std::function<void(EvalState&)>&);
				void endSuite();

				//void require(const std::string&, const std::vector<const std::string&>&);

				void require(const std::string&, const std::string&);
				void require_eval(const std::string&, const std::string&);
				void require_size(const std::string&, size_t);
				void require_type(const std::string&, const std::string&);
				void require_error(const std::string&);
		};

		void run_tests(EvalState&);
	}
}