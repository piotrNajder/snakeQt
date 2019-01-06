#ifndef C_OUTOFBOUNDEXCEPTION_H
#define C_OUTOFBOUNDEXCEPTION_H

#include <exception>
#include <string>

class OutOfBoundException : public std::exception
{
public:
	std::string s;
	OutOfBoundException(std::string ss) : s(ss) {}
	~OutOfBoundException() throw () {}
	const char* what() const throw() { return s.c_str(); }
};

#endif