#pragma once

#include "DustDevFramework.h"

class DustLexer : public DustDev {		// Should I create a generalized log system for testing ???
	private:

	public:

		DustLexer& run(std::string);
		void log(std::ofstream&);
};