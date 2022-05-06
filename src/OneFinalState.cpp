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
#include "OneFinalState.h"
#include "Atomix.h"
#include "Statistics.h"
#include "AStar.h"
#include "Print.h"
#include "StaticPDB.h"
#include "DynamicPDB.h"
#include <limits>
#include <ciso646>
#include <algorithm>

using namespace std;

State single_final_state;
int single_final_state_index = 0;

int calc_initial_heuristic() {
	int initial_max_moves = numeric_limits<int>::max();
	for (auto& f : final_states) {
		initial_max_moves = min(initial_max_moves,
			f.standard_heuristic(initial_state));
	}

#if ParamPDB == PDBDynamic
	int pdb_heuristic = numeric_limits<int>::max();
	for (int i = 0; i < NumFinalStates; ++i) {
		pdb_heuristic = min(pdb_heuristic,
			(int)pdb[i].heuristic_matching(initial_state));
	}
	initial_max_moves = max(initial_max_moves, pdb_heuristic);
#elif ParamPDB == PDBMultiGoal
	initial_max_moves = max(initial_max_moves,
		(int)pdb.heuristic_matching(initial_state));
#elif ParamPDB == PDBStatic
	int pdb_heuristic = numeric_limits<int>::max();
	for (int i = 0; i < NumFinalStates; ++i)
		pdb_heuristic = min(pdb_heuristic, (int)
			static_pdb_heuristic(initial_state, i));
	initial_max_moves = max(initial_max_moves, pdb_heuristic);
#endif
	return initial_max_moves;
}

vector<State> one_final_state() {
	int moves = calc_initial_heuristic();
	for (; not termination_requested; ++moves) {
		println("One final state moves: ", moves);
		stat_lower_bound = moves;
		for (int i = 0; i < NumFinalStates; ++i) {		
			auto& f = final_states[i];
			single_final_state_index = i;
			single_final_state = f;
			if (termination_requested) break;
			int old_nodes_generated = stat_nodes_generated;
			auto sol = a_star(moves);
			stat_nodes_generated_at_depth[moves] += stat_nodes_generated 
				- old_nodes_generated;
			if (sol.size()) return sol;
		}
	}	
	return{};
}
