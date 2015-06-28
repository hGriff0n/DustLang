#pragma once

#include "DustDevFramework.h"

// This is a very basic interpreter for testing purposes.
// This is not the final version that will be integrated into the public api
// The final version will likely be the backend

class DustInterpreter : public DustDev {
	private:
	public:
		DustInterpreter& run(std::string);
		void log(std::ofstream&);
};