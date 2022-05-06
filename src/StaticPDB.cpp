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
#include "StaticPDB.h"
#include "Print.h"
#include "Atomix.h"
#include "RandomNumberGenerator.h"
#include "Statistics.h"
#include "Definitions.h"
#include <cmath>
#include <limits>
#include <queue>
#include <tuple>
#include <ciso646>
#include <cassert>
#include <cstring>
#include <vector>
#include <set>
#define GetPDB3(i, a, b, c) (pdb3[(i)*BoardSize*BoardSize*BoardSize + \
	(a)*BoardSize*BoardSize + (b)*BoardSize + (c)])
#define GetPDB2(a, b) (pdb2[(a)*BoardSize + (b)])
#define GetPDB1(a) (pdb1[(a)])
 
using namespace std;

#if ParamPDB == PDBStatic
StaticPDB pdb[NumFinalStates];
StaticPDB random_pdb[NumFinalStates][MyMax(1, NumRandomStaticPDBs)];
int random_pdb_usage[1 + MyMax(1, NumRandomStaticPDBs)];

void init_static_pbds() {
	memset(random_pdb_usage, 0, sizeof(random_pdb_usage));
#if NumRandomStaticPDBs == 0
	create_random_static_pdbs_greedy(1);	
	for (int f = 0; f < NumFinalStates; ++f) {
		memcpy(&pdb[f], &random_pdb[f][0], sizeof(StaticPDB));
	}
#else 
#if ParamRandomStaticPDB == RandomStaticPDBGreedy
	create_random_static_pdbs_greedy();
#elif ParamRandomStaticPDB == RandomStaticPDBRandom
	create_random_static_pdbs_random();
#endif 	
#endif	
}

void create_random_static_pdbs_greedy(int num_pdbs_to_create) {
	println("create_random_static_pdbs_greedy");
	RandomNumberGenerator<> rng;
#if NumRandomStaticPDBs == 0
	rng.seed(24031992);
#else 
#if ParamRandomSeed != -1
	rng.seed(ParamRandomSeed);
#else 
	rng.randomize();
#endif 
#endif 

	using Perm = vector<vector<int>>;
	set<Perm> P;

	auto sort_perm = [](Perm& p) {
		sort(p.begin(), p.end(), 
			[](const vector<int>& a, const vector<int>& b) {
			if (a.size() == b.size()) return a < b;
			else return a.size() > b.size();			
		});
	};

	for (int i = 0; i < 5000; ++i) {
		vector<int> r(NumAtoms);
		for (int j = 0; j < NumAtoms; ++j) r[j] = j;
		shuffle(begin(r), end(r), rng.engine());
		Perm p((int)ceil(NumAtoms/3.0));
		for (int j = 0; j < NumAtoms; ++j) {
			p[j / 3].push_back(r[j]);
		}
		for (auto& i : p) sort(begin(i), end(i));
		sort_perm(p);		
		P.insert(p);
	}

	println("P.size(): ", P.size());

	auto fitness = [&](const Perm& p) {
		int ans = 0;
		auto& F = final_states[0];
		for (int i = 0; i < (int)p.size(); ++i) {
			for (int j = 0; j < (int)p[i].size();++j) {
				for (int k = j + 1; k < (int)p[i].size(); ++k) {
					ans += relaxed_distances[F.v[p[i][j]]][F.v[p[i][k]]];
				}
			}
		}
		return ans;
	};

	set<pair<int, Perm>> P2;

	for (auto p : P) {
		auto f = fitness(p);
		int iterations = 0;
	LabelLocalSearchStart:
		++iterations;
		if (iterations > 500) continue;
		for (int g1 = 0; g1 < (int)p.size(); ++g1) {
			for (int g2 = g1 + 1; g2 < (int)p.size(); ++g2) {
				for (int a = 0; a < (int)p[g1].size(); ++a) {
					for (int b = 0; b < (int)p[g2].size(); ++b) {
						auto p2 = p;
						swap(p2[g1][a], p2[g2][b]);
						sort(begin(p2[g1]), end(p2[g1]));
						sort(begin(p2[g2]), end(p2[g2]));
						sort_perm(p2);
						auto f2 = fitness(p2);
						if (f2 < f) {
							f = f2;
							p = p2;
							goto LabelLocalSearchStart;
						}
					}
				}
			}
		}
		P2.emplace(f, p);
	}
	
	println("P2.size(): ", P2.size());

	vector<pair<int, Perm>> P3;
	for (auto& p : P2) P3.push_back(p);

	// Complete to NumRandomStaticPDBs with copies of the same PDB

	while ((int)P3.size() < num_pdbs_to_create)
		P3.push_back(P3[0]);

	println("P3.size(): ", P3.size());

	for (int i = 0; i < num_pdbs_to_create; ++i) {
		for (auto& g : P3[i].second) for (auto i : g) print(i, " ");
		print(" - ", P3[i].first);
		print("\n");
	}

	for (int f = 0; f < NumFinalStates; ++f) {
		for (int i = 0; i < num_pdbs_to_create; ++i) {
			auto& p = P3[i].second;

			memset(random_pdb[f][i].atom_group, -1, 
				sizeof(random_pdb[f][i].atom_group));

			int g3_index = 0, g3_count = 0;
			for (int g = 0; g < (int)p.size(); ++g) {
				if (p[g].size() == 3) {
					for (int k = 0; k < 3; ++k) {
						random_pdb[f][i].group3[g3_index++] = p[g][k];
						random_pdb[f][i].atom_group[p[g][k]] = g3_count;
					}
					++g3_count;
				}
#if Num2Groups > 0
				if (p[g].size() == 2) {
					random_pdb[f][i].group2[0] = p[g][0];
					random_pdb[f][i].group2[1] = p[g][1];
				}
#elif Num1Groups > 0
				if (p[g].size() == 1) {
					random_pdb[f][i].group1 = p[g][0];
				}
#endif 			
			}
			random_pdb[f][i].calculate(final_states[f]);
		}
	}
	println("create_random_static_pdbs_greedy done");
}

void create_random_static_pdbs_random() {
	constexpr auto num_greedy = MyMin(2, NumRandomStaticPDBs);
	create_random_static_pdbs_greedy(num_greedy);
	for (int f = 0; f < NumFinalStates; ++f) {
		for (int i = num_greedy; i < NumRandomStaticPDBs; ++i) {
			random_pdb[f][i].choose_groups_random();
			random_pdb[f][i].calculate(final_states[f]);
		}
	}
}

uchar static_pdb_heuristic(const State& s, int pdb_f_index) {
	stat_pdb_matching_timer.restart();
	uchar ans;
#if NumRandomStaticPDBs == 0
	ans = pdb[pdb_f_index].heuristic(s);
#else 
	ans = 0;
	for (int i = 0; i < NumRandomStaticPDBs; ++i) {
		ans = max(ans, random_pdb[pdb_f_index][i].heuristic(s));
	}
#endif 	

#if PrintRandomStaticPDBUsage and NumRandomStaticPDBs > 0
	for (int i = 0; i < NumRandomStaticPDBs; ++i) {
		if (ans == random_pdb[pdb_f_index][i].heuristic(s)) {
			++random_pdb_usage[i];
			break;
		}
	}
#endif
	stat_pdb_matching_time += stat_pdb_matching_timer.elapsed();

	return ans;
}

void StaticPDB::choose_groups_random() {
	static int a[NumAtoms];
	memset(atom_group, -1, sizeof(atom_group));
	for (int i = 0; i < NumAtoms; ++i) a[i] = i;
	shuffle(begin(a), end(a), RandomNumberGenerator<>::instance().engine());
	int j = 0;
	for (int i = 0; i < Num3Groups * 3; ++i) {
		group3[i] = a[j];
		atom_group[j] = i / 3;
		++j;
	}
#if Num2Groups > 0
	group2[0] = a[j++]; 	
	group2[1] = a[j++];
#elif Num1Groups > 0
	group1 = a[j++];
#endif 
}

uchar StaticPDB::heuristic(const State& s) {
	int ans = 0;
	for (int i = 0; i < Num3Groups; ++i)
		ans += GetPDB3(i, s.v[group3[i * 3]], s.v[group3[i * 3 + 1]], 
			s.v[group3[i * 3 + 2]]);

#if Num2Groups > 0
	ans += GetPDB2(s.v[group2[0]], s.v[group2[1]]);
#elif Num1Groups > 0
	ans += GetPDB1(s.v[group1]);
#endif
	return ans;
}

void StaticPDB::calculate(const State& final_state) {
	pdb3 = new PDBDataType[Num3Groups*BoardSize*BoardSize*BoardSize];
	for (int i = 0; i < Num3Groups * pow(BoardSize, 3); ++i)
		pdb3[i] = numeric_limits<PDBDataType>::max();
#if Num2Groups > 0
	pdb2 = new PDBDataType[BoardSize*BoardSize];
	for (int i = 0; i < BoardSize*BoardSize; ++i)
		pdb2[i] = numeric_limits<PDBDataType>::max();
#elif Num1Groups > 0
	pdb1 = new PDBDataType[BoardSize];
	for (int i = 0; i < BoardSize; ++i)
		pdb1[i] = numeric_limits<PDBDataType>::max();
#endif
	for (int i = 0; i < Num3Groups; ++i) {
		bfs_3(i, group3[i * 3], group3[i * 3 + 1], group3[i * 3 + 2],
			final_state);
	}
#if Num2Groups > 0
	bfs_2(group2[0], group2[1], final_state);
#elif Num1Groups > 0
	for (int i = 0; i < BoardSize; ++i)
		pdb1[i] = relaxed_distances[final_state.v[group1]][i];
#endif
}

void StaticPDB::bfs_3(int group_index, int a, int b, int c, const State& s) {
	queue<tuple<int, int, int>> q;
	auto gba = group_begin[a], gsa = group_size[a],
		gbb = group_begin[b], gsb = group_size[b],
		gbc = group_begin[c], gsc = group_size[c];
	int num_added = 0;
	for (int i = gba; i < gba + gsa; ++i) {
		int fa = s.v[i];
		for (int j = gbb; j < gbb + gsb; ++j) {
			if (i == j) continue;
			int fb = s.v[j];
			for (int k = gbc; k < gbc + gsc; ++k) {
				if (k == i or k == j) continue;
				int fc = s.v[k];
				if (GetPDB3(group_index, fa, fb, fc) ==
					numeric_limits<PDBDataType>::max()) {
					GetPDB3(group_index, fa, fb, fc) = 0;
					q.emplace(fa, fb, fc);
					++num_added;
				}
			}
		}
	}

	int num_expanded = 0;
	while (q.size()) {
		int pa = get<0>(q.front()), pb = get<1>(q.front()),
			pc = get<2>(q.front());
		q.pop();
		++num_expanded;

		auto& cur_dist = GetPDB3(group_index, pa, pb, pc);

		for (int dir : PosDirections) {
			int pa2 = pa, pb2 = pb, pc2 = pc;

			while (true) {
				pa2 += dir;
				if (not pos_valid(pa2) or pa2 == pb or pa2 == pc) break;
				if (GetPDB3(group_index, pa2, pb, pc) > 1 + cur_dist) {
					GetPDB3(group_index, pa2, pb, pc) = 1 + cur_dist;
					q.emplace(pa2, pb, pc);
				}
			}

			while (true) {
				pb2 += dir;
				if (not pos_valid(pb2) or pb2 == pa or pb2 == pc) break;
				if (GetPDB3(group_index, pa, pb2, pc) > 1 + cur_dist) {
					GetPDB3(group_index, pa, pb2, pc) = 1 + cur_dist;
					q.emplace(pa, pb2, pc);
				}
			}

			while (true) {
				pc2 += dir;
				if (not pos_valid(pc2) or pc2 == pb or pc2 == pa) break;
				if (GetPDB3(group_index, pa, pb, pc2) > 1 + cur_dist) {
					GetPDB3(group_index, pa, pb, pc2) = 1 + cur_dist;
					q.emplace(pa, pb, pc2);
				}
			}
		}
	}
}

#if Num2Groups > 0
void StaticPDB::bfs_2(int a, int b, const State& s) {
	queue<tuple<Pos, Pos>> q;

	auto gba = group_begin[a], gsa = group_size[a],
		gbb = group_begin[b], gsb = group_size[b];
	for (int i = gba; i < gba + gsa; ++i) {
		int fa = s.v[i];
		for (int j = gbb; j < gbb + gsb; ++j) {
			if (i == j) continue;
			int fb = s.v[j];
			if (GetPDB2(fa, fb) == numeric_limits<PDBDataType>::max()) {
				GetPDB2(fa, fb) = 0;
				q.emplace(fa, fb);
			}
		}
	}
	while (q.size()) {
		Pos pa = get<0>(q.front()), pb = get<1>(q.front());
		q.pop();

		auto& cur_dist = GetPDB2(pa, pb);
		for (auto dir : PosDirections) {
			Pos pa2 = pa, pb2 = pb;

			while (true) {
				pa2 += dir;
				if (not pos_valid(pa2) or pa2 == pb) break;
				if (GetPDB2(pa2, pb) > 1 + cur_dist) {
					GetPDB2(pa2, pb) = 1 + cur_dist;
					q.emplace(pa2, pb);
				}
			}

			while (true) {
				pb2 += dir;
				if (not pos_valid(pb2) or pb2 == pa) break;
				if (GetPDB2(pa, pb2) > 1 + cur_dist) {
					GetPDB2(pa, pb2) = 1 + cur_dist;
					q.emplace(pa, pb2);
				}
			}
		}
	}
}
#endif 

#endif // ParamPDB == PDBStatic

