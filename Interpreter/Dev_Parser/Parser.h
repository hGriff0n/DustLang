#pragma once

#include "DustDevFramework.h"

class DustParser : public DustDev {
	private:
	public:
		DustParser& run(std::string);
		void log(std::ofstream&);
};