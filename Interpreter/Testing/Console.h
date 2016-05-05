#pragma once
#include <iostream>

#ifdef __linux__
static_assert(false, "Support for console manipulators on Linux currently unimplemented");
#elif _WIN32
#include <Windows.h>
#elif __APPLE__
static_assert(false, "Support for console manipulators on Mac OSX currently unimplemented");
#else
static_assert(false, "Unsupported System");
#endif


namespace console {

	// Enable coloring of console text (windows only currently)
	struct color {
		int col;
		constexpr color(int c) : col{ c } {}
	};

	constexpr color black{ 0 }, brown{ 6 }, gray{ 7 }, blue{ 9 }, green{ 10 }, cyan{ 11 }, red{ 12 }, magenta{ 13 }, yellow{ 14 }, white{ 15 };
	constexpr color dark_blue{ 1 }, dark_green{ 2 }, dark_cyan{ 3 }, dark_red{ 4 }, dark_magenta{ 5 }, dark_gray{ 8 };

	// Need to find a way to generalize (for std::cerr) 
	struct color_stream {
		private:
			std::ostream& s;
			CONSOLE_SCREEN_BUFFER_INFO csbi;

		public:
			explicit color_stream(int color);
			explicit color_stream(int color, CONSOLE_SCREEN_BUFFER_INFO b);
			~color_stream();

			// Forward to the underlying stream
			template <typename Rhs>
			friend color_stream& operator<<(color_stream& c, Rhs const& rhs) {
				c.s << rhs;
				return c;
			}

			// Switch color streams
			friend color_stream& operator<<(color_stream& s, color_stream& c);
			friend color_stream operator<<(color_stream& s, const color& c);
	};

	// Switch from ostream to color_stream (move to std ???)
	color_stream& operator<<(std::ostream& s, color_stream& c);
	color_stream operator<<(std::ostream& s, const color& c);

	std::ostream& colorStream(std::ostream& s, const color& c);
}