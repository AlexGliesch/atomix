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
#include "DynamicPDB.h"
#include "Parameters.h"
#include "Atomix.h"
#include "Statistics.h"
#include "Definitions.h"
#include <algorithm>
#include <queue>
#include <limits>
#include <ciso646>
#include <cstring>
#include <cassert>
#define GetPDB(a, b, c, d) (pdb[(int)(d) + (int)(c)*BoardSize + \
	(int)(b)*BoardSize*BoardSize + (int)(a)*BoardSize*BoardSize*NumAtoms])

#if ParamPDB == PDBMultiGoal
DynamicPDB pdb;
#elif ParamPDB == PDBDynamic
DynamicPDB pdb[NumFinalStates];
#endif

using namespace std;

#if ParamPDB == PDBDynamic || ParamPDB == PDBMultiGoal

void DynamicPDB::calculate(State* seeds, int num_seeds) {
#if ParamPDB == PDBMultiGoal
	assert(this->seeds == nullptr);
	assert(this->pdb == nullptr);
	this->num_seeds = num_seeds;
	this->seeds = new State[num_seeds];
	memcpy(this->seeds, seeds, num_seeds * sizeof(State));
#elif ParamPDB == PDBDynamic
	assert(num_seeds == 1);
	seed = seeds[0];
#endif

	constexpr int PDBSize = NumAtoms * NumAtoms * BoardSize * BoardSize;
	pdb = new PDBDataType[PDBSize];
	memset(pdb, -1, PDBSize * sizeof(PDBDataType));
	assert(pdb[0] == numeric_limits<PDBDataType>::max());

	static bool done[NumAtoms][NumAtoms];
	memset(done, false, sizeof(done));

	for (int a = 0; a < NumAtoms; ++a) {
		for (int b = a + 1; b < NumAtoms; ++b) {
			int gba = group_begin[a], gbb = group_begin[b];
			if (done[gba][gbb]) continue;
			done[gba][gbb] = done[gbb][gba] = true;
			queue<pair<int, int>> q;

			for (int i = gba; i < gba + group_size[a]; ++i) {
				for (int j = gbb; j < gbb + group_size[b]; ++j) {
					if (i == j) continue;
					for (int s = 0; s < num_seeds; ++s) {
						int fa = seeds[s].v[i], fb = seeds[s].v[j];
						if (GetPDB(gba, gbb, fa, fb) ==
							numeric_limits<PDBDataType>::max()) {
							assert(GetPDB(gbb, gba, fb, fa) ==
								numeric_limits<PDBDataType>::max());
							q.emplace(fa, fb);
							GetPDB(gba, gbb, fa, fb) = 0;
							GetPDB(gbb, gba, fb, fa) = 0;
						}
					}
				}
			}

			while (q.size()) {
				int pa = q.front().first, pb = q.front().second;
				q.pop();
				for (auto dir : PosDirections) {
					int pa2 = pa, pb2 = pb;
					while (true) {
						pa2 += dir;
						if (pa2 == pb or not pos_valid(pa2)) break;
						int next_score = 1 + GetPDB(gba, gbb, pa, pb);
						if (GetPDB(gba, gbb, pa2, pb) > next_score) {
							GetPDB(gba, gbb, pa2, pb) = next_score;
							GetPDB(gbb, gba, pb, pa2) = next_score;
							q.emplace(pa2, pb);
						}							
					}
					while (true) {
						pb2 += dir;
						if (pb2 == pa or not pos_valid(pb2)) break;
						int next_score = 1 + GetPDB(gba, gbb, pa, pb);
						if (GetPDB(gba, gbb, pa, pb2) > next_score) {
							GetPDB(gba, gbb, pa, pb2) = next_score;
							GetPDB(gbb, gba, pb2, pa) = next_score;
							q.emplace(pa, pb2);
						}
					}
				}
			}			
		}
	}
	initialize_matching();
}

void DynamicPDB::initialize_matching() {
	pm_num_nodes = NumAtoms;
	if (pm_num_nodes % 2 != 0) ++pm_num_nodes;
	assert(pm_weights == nullptr and pm_edges == nullptr);

	const int max_pm_edges = pm_num_nodes * (1 + pm_num_nodes);

	pm_weights = new int[max_pm_edges];
	pm_edges = new int[max_pm_edges * 2]; /* times 2 because front and back */
	memset(pm_weights, 0, max_pm_edges * sizeof(int));	
	memset(pm_edge_index, -1, sizeof(pm_edge_index));

	int counter = 0;
	for (int a = 0; a < NumAtoms; ++a) {
		for (int b = a + 1; b < NumAtoms; ++b) {
			pm_edges[counter * 2] = a;
			pm_edges[counter * 2 + 1] = b;
			pm_edge_index[a][b] = pm_edge_index[b][a] = counter;
			++counter;
		}
		if (NumAtoms % 2 != 0) {
			pm_edge_index[a][NumAtoms] = pm_edge_index[NumAtoms][a] = counter;
			pm_edges[counter * 2] = a;
			pm_edges[counter * 2 + 1] = NumAtoms;
			++counter;
		}
	}
	assert(counter == (pm_num_nodes * (pm_num_nodes - 1)) / 2);

	pm_num_edges = counter;
	assert(pm == nullptr);
	pm = new PerfectMatching(pm_num_nodes, pm_num_edges);
	for (int i = 0; i < pm_num_edges; ++i) {
		pm->AddEdge(pm_edges[2 * i], pm_edges[2 * i + 1], pm_weights[i]);
	}
	pm->options.verbose = false;
	pm->Solve();
}

uchar DynamicPDB::heuristic_matching(const State& state) {
	stat_pdb_update_timer.restart();
	pm->StartUpdate();
	int counter = 0;
	for (int a = 0; a < NumAtoms; ++a) {
		for (int b = a + 1; b < NumAtoms; ++b) {
			int w = -GetPDB(group_begin[a], group_begin[b], 
				state.v[a], state.v[b]);
			pm->UpdateCost(counter, w - pm_weights[counter]);
			pm_weights[counter] = w;
			++counter;
		}
#if NumAtoms % 2 != 0
		int w = -min_distance_of_state_atom_to_final_states(a, state);
		pm->UpdateCost(counter, w - pm_weights[counter]);
		pm_weights[counter] = w;
		++counter;
#endif 
	}
	pm->FinishUpdate();
	stat_pdb_update_time += stat_pdb_update_timer.elapsed();

	stat_pdb_matching_timer.restart();
	pm->Solve();
	int ans = 0, counted_edges = 0;
	for (int i = 0; i < pm_num_edges and 
		counted_edges < (NumAtoms + 1) / 2; ++i) {
		if (pm->GetSolution(i)) {
			ans += pm_weights[i];
			++counted_edges;
		}
	}
	stat_pdb_matching_time += stat_pdb_matching_timer.elapsed();
	ans = -ans;
	assert(ans >= 0 and ans < numeric_limits<uchar>::max());
	return (uchar)ans;
}

int DynamicPDB::min_distance_of_state_atom_to_final_states(int a, 
	const State& state) {
	int w = numeric_limits<int>::max();
#if ParamPDB == PDBMultiGoal
	for (int s = 0; s < num_seeds; ++s) {
		auto& f = seeds[s];
		for (int i = 0; i < group_size[a]; ++i) {
			w = min(w, relaxed_distances[state[a]][f[group_begin[a] + i]]);
		}
	}
#elif ParamPDB == PDBDynamic
	for (int i = 0; i < group_size[a]; ++i) {
		w = min(w, relaxed_distances[state.v[a]][seed.v[group_begin[a] + i]]);
	}
#endif
	return w;
}

#endif // ParamPDB == PDBDynamic || ParamPDB == PDBMultiGoals