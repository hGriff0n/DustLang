#pragma once

#include <stdexcept>

// File for basic exception classes and facilities for dealing with exceptions

namespace dust {
	namespace error {

		class base : public std::exception {
			private:
				const std::string msg;

			public:
				base(const std::string& _m) : msg{ _m } {}
				~base() throw() {}
				virtual const char* what() const throw() { return msg.c_str(); }
		};

	}
}