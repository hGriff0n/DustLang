#include "Console.h"

namespace console {
	color_stream::color_stream(int color) : s{ std::cout } {
		HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
		GetConsoleScreenBufferInfo(h, &csbi);
		SetConsoleTextAttribute(h, color);
	}

	color_stream::color_stream(int color, CONSOLE_SCREEN_BUFFER_INFO b) : s{ std::cout } {
		csbi = b;
		HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(h, color);
	}

	color_stream::~color_stream() {
		HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(h, csbi.wAttributes);
	}

	color_stream& operator<<(color_stream& s, color_stream& c) {
		c.csbi = s.csbi;
		return c;
	}

	color_stream operator<<(color_stream& s, const color& c) {
		return color_stream{ c.col, s.csbi };
	}

	color_stream& operator<<(std::ostream& s, color_stream& c) {
		return c;
	}
	
	color_stream operator<<(std::ostream& s, const color& c) {
		return color_stream{ c.col };
	}
	
	std::ostream & colorStream(std::ostream & s, const color & c) {
		HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(h, c.col);
		return s;
	}
}