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
#include "PDB.h"
// #pragma GCC diagnostic push 
// #pragma GCC diagnostic ignored "-pedantic"
// #pragma GCC diagnostic ignored "-Werror"
#include "BlossomMatching/PerfectMatching.h"
// #pragma GCC diagnostic pop

struct DynamicPDB {
	void calculate(State* seeds, int num_seeds);
	void calculate(State seed) { calculate(&seed, 1); }

	void initialize_matching();

	uchar heuristic_matching(const State& state);

	int min_distance_of_state_atom_to_final_states(int a, const State& state);

	PDBDataType* pdb = nullptr;

#if ParamPDB == PDBMultiGoal
	State* seeds = nullptr;
	int num_seeds = 0;
#elif ParamPDB == PDBDynamic
	State seed;
#endif 
	
	int pm_edge_index[1 + NumAtoms][1 + NumAtoms];

	/* blossom matching stuff */
	class PerfectMatching* pm = nullptr;
	int* pm_weights = nullptr, *pm_edges = nullptr;
	int pm_num_nodes = 0, pm_num_edges = 0;
};

#if ParamPDB == PDBMultiGoal
extern DynamicPDB pdb;
#elif ParamPDB == PDBDynamic
extern DynamicPDB pdb[NumFinalStates];
#endif

