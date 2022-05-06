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
#include "State.h"
#include "Parameters.h"

#if ParamHeuristic == HeuAllFinalStates
void tie_breaking(State& s, int f);
#elif ParamHeuristic == HeuOneFinalState
void tie_breaking(State& s);
#endif

void tie_breaking_delta(State& new_state, State& old_state, 
	int atom_moved);

#if ParamHeuristic == HeuAllFinalStates
int tb_goal_count(State& s, int f);
int tb_goal_count(State& s);
#elif ParamHeuristic == HeuOneFinalState
int tb_goal_count(State& s);
#endif
int tb_goal_count_delta(State& new_state, State& old_state,
	int atom_moved);

#if ParamHeuristic == HeuAllFinalStates
int tb_fill_order(State& s, int f);
#elif ParamHeuristic == HeuOneFinalState
int tb_fill_order(State& s);
#endif
int tb_fill_order_delta(State& new_state, State& old_state,
	int atom_moved);

#if ParamHeuristic == HeuAllFinalStates
int tb_nrp(State& s, int f);
#elif ParamHeuristic == HeuOneFinalState
int tb_nrp(State& s);
#endif

int nrp_bfs(State& s, int f, int a);

void compute_fill_order_ranks();
extern int fill_order_ranks[NumFinalStates][BoardSize];
extern int max_fill_order;

#define RandomTBRangeSize 250

#if ParamTieBreaking == TBGoalCount
	#define MaxTieBreakingValue NumAtoms
#elif ParamTieBreaking == TBNumberRealizablePaths
	#define MaxTieBreakingValue NumAtoms
#elif ParamTieBreaking == TBFillOrder || ParamTieBreaking == TBFillOrderReverse
	#define MaxTieBreakingValue max_fill_order
#elif ParamTieBreaking == TBRandom
	#define MaxTieBreakingValue (RandomTBRangeSize-1)
#elif ParamTieBreaking == TBH
	#define MaxTieBreakingValue GuessOnMaximumFValue
#elif ParamTieBreaking == TBHGoalCount || ParamTieBreaking == TBGoalCountH
	#define MaxTieBreakingValue ((1+GuessOnMaximumFValue)*(1+NumAtoms))
#else 
	#define MaxTieBreakingValue 1
#endif