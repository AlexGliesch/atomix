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
#include "State.h"
#include "Print.h"
#include "Atomix.h"
#include "Board.h"
#include "Definitions.h"
#include "MinCostBipartiteMatching.h"
#include <cassert>

using namespace std;

bool State::contains(Pos p) const {
	for (int i = 0; i < StateSize; ++i) {
		if (v[i] == p) return true;
	}
	return false;
}

void State::pretty_print(ostream& o) const {
	string b(board);
	for (int i = 0; i < NumAtoms; ++i)
		b[v[i]] = index_to_label[i];
	for (int r = 0; r < BoardHeight; ++r) {
		for (int c = 0; c < BoardWidth; ++c)
			print_stream(o, b[pos(r, c)]);
		print_stream(o, '\n');
	}
}

int State::standard_heuristic(const State& d) const {
	int x = 0;

	/* atoms with single atomicity */
	for (int a = 0; a < multi_start_index; ++a)
		x += relaxed_distances[v[a]][d.v[a]];

	/* atoms with multiple atomicity: matching */
	for (int a = multi_start_index; a < NumAtoms; a += group_size[a]) {
		x += multi_atom_matching(d, a);
	}
	return x;
}

int State::atom_standard_heuristic(const State& d, int a) const {
	return (a < multi_start_index) ? 
		relaxed_distances[v[a]][d.v[a]] :
		multi_atom_matching(d, group_begin[a]);
}

int State::multi_atom_matching(const State& d, int a) const {
	auto& dist = relaxed_distances;
	if (group_size[a] == 2) {
		int i1 = dist[v[a]][d.v[a]] + dist[v[a + 1]][d.v[a + 1]];
		int i2 = dist[v[a + 1]][d.v[a]] + dist[v[a]][d.v[a + 1]];
		return min(i1, i2);
	} else if (group_size[a] == 3) {
		int i1 = dist[v[a]][d.v[a]] + dist[v[a + 1]][d.v[a + 1]] +
			dist[v[a + 2]][d.v[a + 2]];
		int i2 = dist[v[a]][d.v[a]] + dist[v[a + 1]][d.v[a + 2]] +
			dist[v[a + 2]][d.v[a + 1]];
		int i3 = dist[v[a]][d.v[a + 1]] + dist[v[a + 1]][d.v[a]] +
			dist[v[a + 2]][d.v[a + 2]];
		int i4 = dist[v[a]][d.v[a + 1]] + dist[v[a + 1]][d.v[a + 2]] +
			dist[v[a + 2]][d.v[a]];
		int i5 = dist[v[a]][d.v[a + 2]] + dist[v[a + 1]][d.v[a]] +
			dist[v[a + 2]][d.v[a + 1]];
		int i6 = dist[v[a]][d.v[a + 2]] + dist[v[a + 1]][d.v[a + 1]] +
			dist[v[a + 2]][d.v[a]];
		return min(min(i1, i2), min(min(i3, i4), min(i5, i6)));
	} else {
		/* larger than threshold, use matching */
		if (termination_requested) return 0;

		assert(a == group_begin[a]);
		int gsa = group_size[a];

		static int m[NumAtoms][NumAtoms];
		static int Rmate[NumAtoms], Lmate[NumAtoms];
		for (int i = 0; i < gsa; ++i) {
			for (int j = 0; j < gsa; ++j) {
				m[i][j] = relaxed_distances[v[a + i]][d.v[a + j]];
			}
		}
		return min_cost_bipartite_matching(m, Lmate, Rmate, gsa);
	}
}

bool State::is_obstacle(Pos p) const {
	if (board_is_wall(p)) return true;
	for (int i = 0; i < NumAtoms; ++i) {
		if (v[i] == p) return true;
	}
	return false;
}