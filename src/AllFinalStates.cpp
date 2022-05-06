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
#include "AllFinalStates.h"
#include "OneFinalState.h"
#include "Atomix.h"
#include "AStar.h"
#include "Print.h"
#include "Statistics.h"
#include <ciso646>

std::vector<State> all_final_states() {
#if ParamAlgorithm == AlgLayeredAStar
	return all_final_states_layered();
#else 
	return a_star();
#endif 
}

std::vector<State> all_final_states_layered() {
	int moves = calc_initial_heuristic();
	for (; not termination_requested; ++moves) {		
		println("All final state moves: ", moves);
		stat_lower_bound = moves;
		int old_nodes_generated = stat_nodes_generated;
		auto sol = a_star(moves);
		stat_nodes_generated_at_depth[moves] += stat_nodes_generated
			- old_nodes_generated;
		if (sol.size()) return sol;
	}
	return{};
}

