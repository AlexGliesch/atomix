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
#include "Pos.h"
#include "Parameters.h"
#include "Board.h"
#include "Definitions.h"
#include "Print.h"
#include <limits>
#include <ciso646>
#include <cmath>

using namespace std;

int PosDirections[4] = { -BoardWidth, -1, BoardWidth, 1 };

Pos pos(int r, int c) {
	return r * BoardWidth + c;
}

int pos_r(Pos p) {
	return p / BoardWidth;
}

int pos_c(Pos p) {
	return p % BoardWidth;
}

bool pos_out_of_bounds(int p) {
	return (p < 0 or p >= BoardSize);
}

bool pos_in_bounds(int p) {
	return (p >= 0 and p < BoardSize);
}

int manhattan_distance(Pos a, Pos b) {
	return std::abs(pos_r(a) - pos_r(b)) + std::abs(pos_c(a) - pos_c(b));
}

void pos_pretty_print(int p) {
	print(pos_str(p));
}

string pos_str(int p) {
	return "(" + to_string(pos_r(p)) + ", " + to_string(pos_c(p)) + ")";
} 

bool pos_valid(int x) {
	if (x >= 0 and x < BoardSize)
		return board[x] != '#';
	return false;
}
