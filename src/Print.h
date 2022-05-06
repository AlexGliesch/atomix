/*
* Solving Atomix with pattern databases
* Copyright (c) 2016 Alex Gliesch, Marcus Ritt
*
* Permission is hereby granted, free of charge, to any person (the "Person")
* obtaining a copy of this software and associated documentation files (the
* "Software"), to deal in the Software, including the rights to use, copy, modify,
* merge, publish, distribute the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* 1. The above copyright notice and this permission notice shall be included in
*    all copies or substantial portions of the Software.
* 2. Under no circumstances shall the Person be permitted, allowed or authorized
*    to commercially exploit the Software.
* 3. Changes made to the original Software shall be labeled, demarcated or
*    otherwise identified and attributed to the Person.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
* FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
* COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
* IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
* CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#pragma once
#include "Parameters.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <locale>
#include <ctime>
#include <chrono>

template<typename T>
std::string to_string(T val) {
	std::ostringstream ss;
	ss << val;
	return ss.str();
}

template<typename Head>
void print(const Head& h) {
#if !ParamSilent
	std::cout << h;
#endif
}

template<typename Head, typename... Tail>
void print(const Head& h, Tail... tail) {
#if !ParamSilent
	print(h);
	print(tail...);
#endif
}

template<typename Head>
void println(const Head& h) {
#if !ParamSilent
	std::cout << h << std::endl;
#endif
}

template<typename Head, typename... Tail>
void println(const Head& h, Tail... tail) {
#if !ParamSilent
	print(h);
	print(tail...);
	print('\n');
#endif
}

template<typename Head>
void print_stream(std::ostream& os, const Head& h) {
	os << h;
}

template<typename Head, typename... Tail>
void print_stream(std::ostream& os, const Head& h, Tail... tail) {
	print_stream(os, h);
	print_stream(os, tail...);
}

template<typename Head>
void println_stream(std::ostream& os, const Head& h) {
	os << h << '\n';
}

template<typename Head, typename... Tail>
void println_stream(std::ostream& os, const Head& h, Tail... tail) {
	print_stream(os, h);
	print_stream(os, tail...);
	print_stream(os, '\n');
}

struct comma_numpunct : public std::numpunct<char> {
protected:
	virtual char do_thousands_sep() const {
		return ',';
	}

	virtual std::string do_grouping() const {
		return "\03";
	}
};