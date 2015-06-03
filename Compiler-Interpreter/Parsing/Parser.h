#pragma once

#include <string>

#define DustExeTmp DustChunk
#define DustParseTmp DustParser

class DustExeTmp {};

class DustParseTmp {			// Or should this be a lexer ???
	private:
		dust::ParsingState curr;
		std::string currCodeFile;

	public:
		DustParseTmp();
		DustParseTmp(std::string);

		DustExeTmp parse();
		DustExeTmp parse(std::string& code);				// This is a useful function to have, but I think the other is better

		/*
		Function parse that performs some degree of parsing and then returns an executable object that will update the dust state to reflect the code
		This function can be called repeatedly on a section of code and each step will return a different executable (could we have an iterator pattern ???)

		DustState ds;
		for (auto chunk : DustParser("hello.ds"))			// so what would import/cmd-line interpreter entail
			ds.run(chunk);
		*/
};

namespace dust {

	class ParsingState {};		// Base state for the finite automata (or should this be the "controller")

	class InitialState : public ParsingState {};

}