#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

struct DustDev {
	virtual DustDev& run(std::string);
	virtual void log(std::ofstream&);
};

