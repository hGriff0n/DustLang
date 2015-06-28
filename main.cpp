//#include "Dust.h"
//#include <iostream>


#include "Lexer/Lexer.h"
//#include "Parsers/Parser.h"
//#include "Interpreter/Interpreter.h"

#define DUST_STAGE DustLexer
#define bad_error(x)	\
		std::cout << x;	\
		std::cin.get();	\
		exit(0)

std::vector<std::ofstream> getLogs(std::_Iosb<int>::_Openmode open) {
	std::vector<std::ofstream> ret{};

	int num_logs = 10;
	//std::cout << "Num logs: ";
	//std::cin >> num_logs;

	char curr = '0';
	std::string def = "DustTestLog";

	for ( int x = 0; x != num_logs; ++x )
		ret.emplace_back(def + (curr++) + ".txt", open);			// the current system only allows 10 logs (there's not character '10')

	return ret;
}

std::string filename(std::string input) {
	return input;
}

template <typename T>
void advance(int& idx, std::vector<T>& vec) {
	idx = (++idx) % vec.size();
}


// LNK 2019: unresolved external symbol _WinMain@16 referenced in function :int __cdecl invoke_main(void)" (?invoid_main@@YAHXZ)
int main(int argc, const char* argv[]) {
	// Setup logging system
	std::vector<std::ofstream> logs = getLogs(std::ios::trunc);
	for ( auto& log : logs )
		if ( !log.is_open() ) {
			bad_error("Error opening log file");	// Need way of getting the specific log file
		}


	// Setup testing objects and repl loop
	DustDev tester = DUST_STAGE{};
	std::string input;
	int curr_log = 0;


	// Testing repl loop
	while ( true ) {
		std::cout << curr_log << "> ";
		std::getline(std::cin, input);
		
		if ( input == "exit" ) break;
		tester.run(filename(input)).log(logs[curr_log]);

		advance(curr_log, logs);
	}

	std::cout << "\nThe repl loop has been exited.";
}