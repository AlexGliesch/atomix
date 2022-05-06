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
#include "Atomix.h"
#include "Parameters.h"
#include "Board.h"
#include "Print.h"
#include "TieBreaking.h"
#include "StatesTable.h"
#include "DynamicPDB.h"
#include "StaticPDB.h"
#include "Statistics.h"
#include <cctype>
#include <cstring>
#include <cassert>
#include <algorithm>
#include <vector>
#include <ciso646>
#include <queue>

using namespace std;

bool termination_requested = false;
size_t current_memory = 0;

int num_atom_types = 0;
int group_size[NumAtoms], group_begin[NumAtoms];
int num_multi_atoms = 0, multi_start_index = 0;
char index_to_label[256];
int label_to_index[256];

State initial_state;
State final_states[NumFinalStates];

int relaxed_distances[BoardSize][BoardSize];

void preprocess() {
	memcpy(board, ParamBoard, BoardSize * sizeof(char));

	pair<unsigned char, vector<Pos>> atom_positions[256];

	for (int i = 0; i < 256; ++i) atom_positions[i].first = i;

	for (int p = 0; p < BoardSize; ++p) {
		if (board_is_atom(p)) {
			atom_positions[(int)board[p]].second.push_back(p);
		}
	}

	sort(begin(atom_positions), end(atom_positions), [&](
		const pair<unsigned char, vector<Pos>>& a,
		const pair<unsigned char, vector<Pos>>& b) {
		return a.second.empty() ? false : a.second.size() < b.second.size();
	});

	memset(label_to_index, -1, sizeof(label_to_index));
	memset(index_to_label, -1, sizeof(index_to_label));

	int atom_count = 0;
	for (const auto& p : atom_positions) {
		if (p.second.empty()) continue;
		label_to_index[p.first] = atom_count;
		for (int i = 0; i < (int)p.second.size(); ++i) {
			group_begin[atom_count + i] = atom_count;
			group_size[atom_count + i] = p.second.size() - i;
			initial_state.v[atom_count + i] = p.second[i];
		}
		atom_count += p.second.size();
	}

	for (int i = 0; i < 256; ++i) {
		if (label_to_index[i] != -1) {
			for (int j = 0; j < group_size[(int)label_to_index[i]]; ++j) {
				index_to_label[label_to_index[i] + j] = i;
			}		
		}
	}

	multi_start_index = distance(begin(group_size), find_if(begin(group_size),
		end(group_size), [](int i) {
		return i > 1;
	}));

	num_multi_atoms = count_if(begin(group_size), end(group_size),
		[](int i) {
		return i > 1;
	});

	for (int i = 0; i < NumAtoms; ++i)
		group_size[i] = group_size[group_begin[i]];

	num_atom_types = 0;
	for (int i = 0; i < NumAtoms; i += group_size[i])
		++num_atom_types;

	board_remove_atoms();
	board_flood();
	find_final_states();
	compute_relaxed_distances();

#if ParamTieBreaking == TBFillOrder or ParamTieBreaking == TBFillOrderReverse
	compute_fill_order_ranks();
#endif 

#if ParamPDB == PDBMultiGoal
	pdb.calculate(final_states, NumFinalStates);
#elif ParamPDB == PDBDynamic
	for (int i = 0; i < NumFinalStates; ++i)
		pdb[i].calculate(&final_states[i], 1);
#elif ParamPDB == PDBStatic
	init_static_pbds();
#endif

	memset(stat_nodes_generated_at_depth, 0, 
		sizeof(stat_nodes_generated_at_depth));

	memset(stat_nodes_generated_with_f_value, 0,
		sizeof(stat_nodes_generated_with_f_value));
	memset(stat_nodes_expanded_with_f_value, 0,
		sizeof(stat_nodes_expanded_with_f_value));

	memset(stat_nodes_generated_with_g_value, 0,
		sizeof(stat_nodes_generated_with_g_value));
	memset(stat_nodes_expanded_with_g_value, 0,
		sizeof(stat_nodes_expanded_with_g_value));

	memset(stat_nodes_generated_with_h_value, 0,
		sizeof(stat_nodes_generated_with_h_value));
	memset(stat_nodes_expanded_with_h_value, 0,
		sizeof(stat_nodes_expanded_with_h_value));
}

void find_final_states() {
	int final_state_index = 0;
	assert(BoardHeight >= MoleHeight);
	assert(BoardWidth >= MoleWidth);
	for (int r = 0; r < BoardHeight - MoleHeight; ++r) {
		for (int c = 0; c < BoardWidth - MoleWidth; ++c) {
			bool can_place_there = true;
			for (int mr = 0; can_place_there and mr < MoleHeight; ++mr) {
				for (int mc = 0; can_place_there and mc < MoleWidth; ++mc) {
					assert(mc + mr*MoleWidth < MoleSize);
					assert(c + r*BoardWidth < BoardSize);
					if (isalnum(ParamMole[mc + mr*MoleWidth]) and
						board_is_wall(r + mr, c + mc)) {
						can_place_there = false;
					}
				}
			}
			if (can_place_there) {				
				auto& f = final_states[final_state_index];
				for (int mr = 0; mr < MoleHeight; ++mr)
					for (int mc = 0; mc < MoleWidth; ++mc)
						if (isalnum(ParamMole[mc + mr*MoleWidth])) {
							int i = label_to_index[
								(int)ParamMole[mc + mr*MoleWidth]];						
							while (f.v[i] != std::numeric_limits<Pos>::max()) ++i;
							f.v[i] = pos(r + mr, c + mc);							
						}
				++final_state_index;
			}
		}
	}

	if (final_state_index != NumFinalStates) {
		println("Final States found by C++: ", final_state_index);
		println("Final States found by python: ", NumFinalStates);

		exit(-1);
	}
	assert(final_state_index == NumFinalStates);

	/* sort the atoms with duplicates by the lexicographical order of their
	* positions */
	for (int i = multi_start_index; i < NumAtoms; i += group_size[i]) {
		for (auto& s : final_states)
			sort(begin(s.v) + i, begin(s.v) + i + group_size[i]);
	}
}

void compute_relaxed_distances() {
	for (int i = 0; i < BoardSize; ++i)
		for (int j = 0; j < BoardSize; ++j)
			relaxed_distances[i][j] = numeric_limits<int>::max();

	for (int i = 0; i < BoardSize; ++i) {
		auto& dist = relaxed_distances[i];
		queue<Pos> q;
		q.push(i);
		dist[i] = 0;
		while (q.size()) {
			Pos p = q.front(); q.pop();
			for (auto d : PosDirections) {
				for (int n = p + d; pos_valid(n); n += d) {
					if (dist[n] > dist[p] + 1) {
						dist[n] = dist[p] + 1;
						q.push(n);
					}
				}
			}
		}
	}
}
