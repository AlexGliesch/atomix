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
#include "Board.h"
#include "Atomix.h"
#include <queue>
#include <ciso646>
#include <cstring>

using namespace std;

char board[BoardSize];

void board_flood() {
	bool visited[BoardSize]; 
	memset(visited, false, sizeof(visited));
	queue<int> q;
	for (auto p : initial_state.v)
		q.push(p);

	while (q.size()) {
		int p = q.front(); q.pop();
		visited[p] = true;
		for (auto d : PosDirections) {
			if (not pos_valid(p + d)) continue;
			if (not visited[p + d]) {
				q.push(p + d);
			}
		}
	}
	for (int i = 0; i < BoardSize; ++i) {
		if (not visited[i]) board[i] = '#';
	}
}

