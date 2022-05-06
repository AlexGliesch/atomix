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
#include "TieBreaking.h"
#include "OneFinalState.h"
#include "Print.h"
#include "Atomix.h"
#include "Board.h"
#include <cassert>
#include <ciso646>
#include <cstring>
#include <algorithm>
#include <functional>
#include <boost/heap/fibonacci_heap.hpp>
#include "RandomNumberGenerator.h"

using namespace std;

int fill_order_ranks[NumFinalStates][BoardSize];
int max_fill_order = 0;

#if ParamTieBreaking != None

#if ParamHeuristic == HeuAllFinalStates
void tie_breaking(State& s, int f) {
	#if ParamTieBreaking == TBGoalCount
	s.tie_breaker = tb_goal_count(s, f);
	#elif ParamTieBreaking == TBFillOrder
	s.tie_breaker = tb_fill_order(s, f);
	#elif ParamTieBreaking == TBNumberRealizablePaths
	s.tie_breaker = tb_nrp(s, f);
	#elif ParamTieBreaking == TBRandom
	(void)f;
	s.tie_breaker = random_number<int>(0, MaxTieBreakingValue - 1);
	#elif ParamTieBreaking == TBH
	(void)f;
	s.tie_breaker = s.g_value;
	#elif ParamTieBreaking == TBHGoalCount
	s.tie_breaker = s.g_value * NumAtoms + tb_goal_count(s, f);
	#elif ParamTieBreaking == TBGoalCountH
	s.tie_breaker = tb_goal_count(s, f) * GuessOnMaximumFValue + s.g_value;
	#endif
}
#elif ParamHeuristic == HeuOneFinalState
void tie_breaking(State& s) {
	#if ParamTieBreaking == TBGoalCount
	s.tie_breaker = tb_goal_count(s);
	#elif ParamTieBreaking == TBFillOrder
	s.tie_breaker = tb_fill_order(s);
	#elif ParamTieBreaking == TBNumberRealizablePaths
	s.tie_breaker = tb_nrp(s);
	#elif ParamTieBreaking == TBRandom
	s.tie_breaker = random_number<int>(0, MaxTieBreakingValue - 1);
	#elif ParamTieBreaking == TBH
	s.tie_breaker = s.g_value;
	#elif ParamTieBreaking == TBHGoalCount
	s.tie_breaker = s.g_value * NumAtoms + tb_goal_count(s);
	#elif ParamTieBreaking == TBGoalCountH
	s.tie_breaker = tb_goal_count(s) * GuessOnMaximumFValue + s.g_value;
	#endif
}
#endif
	
void tie_breaking_delta(State& new_state, State& old_state, int atom_moved) {
#if ParamTieBreaking == TBGoalCount
	new_state.tie_breaker = tb_goal_count_delta(new_state, old_state, 
		atom_moved);
#elif ParamTieBreaking == TBFillOrder
	new_state.tie_breaker = tb_fill_order_delta(new_state, old_state, 
		atom_moved);
#elif ParamTieBreaking == TBNumberRealizablePaths
	// NRP does not allow for a simple delta updating, since moving an atom
	// can change paths of other atoms from being realizable to not and vice-
	// versa
	#if ParamHeuristic == HeuOneFinalState
	new_state.tie_breaker = tb_nrp(new_state);
	#elif ParamHeuristic == HeuAllFinalStates
	new_state.tie_breaker = tb_nrp(new_state, 0);
	#endif
#elif ParamTieBreaking == TBRandom
	(void)old_state; (void)atom_moved;
	new_state.tie_breaker = random_number<int>(0, MaxTieBreakingValue - 1);
#elif ParamTieBreaking == TBH 
	(void)old_state; (void)atom_moved;
	new_state.tie_breaker = new_state.g_value;

#elif ParamTieBreaking == TBHGoalCount 
	(void)old_state; (void)atom_moved;
	#if ParamHeuristic == HeuAllFinalStates 
	assert(NumFinalStates == 1);
	int gc = tb_goal_count(new_state, 0);
	#elif ParamHeuristic == HeuOneFinalState
	int gc = tb_goal_count(new_state, single_final_state_index);
	#endif 
	new_state.tie_breaker = new_state.g_value * NumAtoms + gc;
#elif ParamTieBreaking == TBGoalCountH 
	(void)old_state; (void)atom_moved; 
	#if ParamHeuristic == HeuAllFinalStates 
	assert(NumFinalStates == 1);
	int gc = tb_goal_count(new_state, 0);
	#elif ParamHeuristic == HeuOneFinalState
	int gc = tb_goal_count(new_state, single_final_state_index);
	#endif 	
	new_state.tie_breaker = gc * GuessOnMaximumFValue + new_state.g_value;
	/*if (new_state.tie_breaker > MaxTieBreakingValue) {
		println("gc = ", gc);
		println("g = ", (int)new_state.g_value);
		println("tie_breaker = ", new_state.tie_breaker);
		println("GuessOnMaximumFValue = ", GuessOnMaximumFValue);
		println("MaxTieBreakingValue = ", MaxTieBreakingValue);		
		exit(1);
	}*/
#endif
}

#if ParamHeuristic == HeuAllFinalStates
int tb_goal_count(State& s) {
	int gc = 0;
	for (int i = 0; i < NumFinalStates; ++i)
		gc = max(gc, (int)tb_goal_count(s, i));
	return gc;
}

int tb_goal_count(State& s, int f)
#elif ParamHeuristic == HeuOneFinalState
int tb_goal_count(State& s)
#endif
{
#if ParamHeuristic == HeuAllFinalStates
	auto& F = final_states[f];
#elif ParamHeuristic == HeuOneFinalState
	auto& F = single_final_state;
#endif
	int score = 0;
	for (int i = 0; i < multi_start_index; ++i) {
		if (s.v[i] == F.v[i])
			++score;
	}
	for (int i = multi_start_index; i < NumAtoms; ++i) {
		for (int j = 0; j < group_size[i]; ++j) {
			if (s.v[i] == F.v[group_begin[i] + j]) {
				++score;
				break;
			}
		}
	}
	return score;
}

int tb_goal_count_delta(State& new_state, State& old_state,
	int atom_moved) {
#if ParamHeuristic == HeuAllFinalStates
	auto& F = final_states[0];
#elif ParamHeuristic == HeuOneFinalState
	auto& F = single_final_state;
#endif
	int old_score = 0, new_score = 0;
	if (atom_moved < multi_start_index) {
		if (F.v[atom_moved] == old_state.v[atom_moved])
			old_score = 1;
		if (F.v[atom_moved] == new_state.v[atom_moved])
			new_score = 1;
	} else {
		auto gb = group_begin[atom_moved];
		for (int i = gb; i < gb + group_size[gb]; ++i) {
			for (int j = gb; j < gb + group_size[gb]; ++j) {
				if (F.v[j] == old_state.v[i])
					++old_score;
				if (F.v[j] == new_state.v[i])
					++new_score;
			}
		}
		assert(old_score <= group_size[gb]);
		assert(new_score <= group_size[gb]);
	}
	return old_state.tie_breaker - old_score + new_score;
}

#if ParamHeuristic == HeuAllFinalStates
int tb_fill_order(State& s, int f)
#elif ParamHeuristic == HeuOneFinalState
int tb_fill_order(State& s)
#endif
{
#if ParamHeuristic == HeuAllFinalStates
	auto& F = final_states[f];
#elif ParamHeuristic == HeuOneFinalState
	auto& F = single_final_state;
	const int f = single_final_state_index;
#endif
	int score = 0;
	for (int i = 0; i < multi_start_index; ++i) {
		if (s.v[i] == F.v[i])
			score += fill_order_ranks[f][s.v[i]];
	}
	for (int i = multi_start_index; i < NumAtoms; ++i) {
		for (int j = 0; j < group_size[i]; ++j) {
			if (s.v[i] == F.v[group_begin[i] + j]) {
				score += fill_order_ranks[f][s.v[i]];
				break;
			}
		}
	}
	return score;
}

int tb_fill_order_delta(State& new_state, State& old_state, int atom_moved) {
#if ParamHeuristic == HeuAllFinalStates
	auto& F = final_states[0];
	const int f = 0;
#elif ParamHeuristic == HeuOneFinalState
	auto& F = single_final_state;
	const int f = single_final_state_index;
#endif
	int old_score = 0, new_score = 0;

	if (atom_moved < multi_start_index) {
		if (F.v[atom_moved] == old_state.v[atom_moved])
			old_score = fill_order_ranks[f][F.v[atom_moved]];
		if (F.v[atom_moved] == new_state.v[atom_moved])
			new_score = fill_order_ranks[f][F.v[atom_moved]];
	} else {
		auto gb = group_begin[atom_moved];
		for (int a = gb; a < gb + group_size[atom_moved]; ++a) {
			for (int i = gb; i < gb	+ group_size[atom_moved]; ++i) {
				if (old_state.v[a] == F.v[i])
					old_score += fill_order_ranks[f][old_state.v[a]];
				if (new_state.v[a] == F.v[i])
					new_score += fill_order_ranks[f][new_state.v[a]];
			}
		}
	}
	assert(old_score <= old_state.tie_breaker);
	return old_state.tie_breaker - old_score + new_score;
}

#if ParamHeuristic == HeuAllFinalStates
int tb_nrp(State& s, int f)
#elif ParamHeuristic == HeuOneFinalState
int tb_nrp(State& s)
#endif
{
#if ParamHeuristic == HeuAllFinalStates
	auto& F = final_states[f];
#elif ParamHeuristic == HeuOneFinalState
	auto& F = single_final_state;
	int f = single_final_state_index;
#endif

	int score = 0;
	for (int i = 0; i < NumAtoms; ++i) {
		auto x = nrp_bfs(s, f, i);
		if (x == relaxed_distances[s.v[i]][F.v[i]])
			++score;
	}
	return score;
}

int nrp_bfs(State& s, int f, int a) {
	// Optimize this function. for instance, on adrienl_01, which is a 
	// relatively small instance (2 million nodes), this is already too costly.
	// We could to an A* here or something else. the problem here is mainly that 
	// it tries to add too many invalid nodes to the queue.	
	auto& F = final_states[f];
	static int dist[BoardSize];
	boost::heap::fibonacci_heap<tuple<int, int, int>, 
		boost::heap::compare<greater<tuple<int, int, int>>>> pq;

	memset(dist, -1, sizeof(dist));
	dist[s.v[a]] = 0;
	pq.emplace(relaxed_distances[F.v[a]][s.v[a]], 0, s.v[a]);

#define Fval(x) (get<0>(x))
#define Gval(x) (get<1>(x))
#define Hval(x) (get<0>(x) - get<1>(x))
#define Vertex(x) (get<2>(x))

	while (not pq.empty()) {
		auto p = pq.top(); pq.pop();

		if (Gval(p) != dist[Vertex(p)])
			continue;
		
		for (int i = group_begin[a]; i < group_begin[a] + 
			group_size[group_begin[a]]; ++i) {
			if (Vertex(p) == F.v[i]) {
				//println("nrp_bfs() done");
				assert(Hval(p) == 0);
				return Gval(p);
			}
		}

		for (int d : PosDirections) {
			auto x = Vertex(p) + d;
			while (pos_valid(x) and not s.contains(x)) {
				if (dist[x] == -1 or dist[x] > 1 + Gval(p)) {
					dist[x] = 1 + Gval(p);
					pq.emplace(dist[x] + relaxed_distances[F.v[a]][x], 
						dist[x], x);
				}
				x += d;
			}
		}
	}
#undef Fval
#undef Gval
#undef Hval
#undef Vertex
	return -1;
}

void compute_fill_order_ranks() {
	memset(fill_order_ranks, 0, sizeof(fill_order_ranks));
	for (int f = 0; f < NumFinalStates; ++f) {
		int level = 1, atoms_removed = 0;
		int removed[BoardSize];
		memset(removed, 0, sizeof(removed));

		auto backward_move_possible = [&](Pos a) {
			auto is_free = [&](Pos p) {
				if (board_is_wall(p)) return false;
				auto c = final_states[f].contains(p);
				return not c or (c and removed[p] and removed[p] != level);
			};

			auto can_be_obstacle = [&](Pos p) {
				return board_is_wall(p) or final_states[f].contains(p);
			};

			for (int d : PosDirections) {
				if (not pos_valid((int)a - d) or not pos_valid((int)a + d))
					continue;
				if (can_be_obstacle((int)a - d) and is_free((int)a + d))
					return true;
			}
			return false;
		};

		while (atoms_removed < NumAtoms) {
			bool removed_something = false;
			for (int i = 0; i < NumAtoms; ++i) {
				Pos a = final_states[f].v[i];
				if (not removed[a] and backward_move_possible(a)) {
					++atoms_removed;
					removed_something = true;
					removed[a] = level;
					fill_order_ranks[f][a] = level;
				}
			}
			/* this will happen when some atoms simply cannot have a backward
			* move (i.e., they are alone in the middle without obstacles) */
			if (not removed_something)
				break;
			++level;
		}
		
		for (Pos a : final_states[f].v) {
			if (not removed[a]) fill_order_ranks[f][a] = level;
		}

		for (int a = 0; a < BoardSize; ++a) {
			if (board_is_wall(a)) continue;
			if (fill_order_ranks[f][a]) {
#if ParamTieBreaking == TBFillOrderReverse
				fill_order_ranks[f][a] = pow(2, level - fill_order_ranks[f][a]);
#elif ParamTieBreaking == TBFillOrder
				fill_order_ranks[f][a] = pow(2, fill_order_ranks[f][a]);
#endif					
			}
		}
		int foF[BoardSize]; 
		memcpy(foF, fill_order_ranks[f], BoardSize * sizeof(int));
		sort(&foF[0], &foF[0] + BoardSize, std::greater<int>());
		int largest = 1;
		for (int i = 0; i < NumAtoms; ++i)
			largest += foF[i];		
		
		max_fill_order = max(max_fill_order, largest);
	}
}

#endif