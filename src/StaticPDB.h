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
#include "PDB.h"
#include "Parameters.h"
#include "Definitions.h"
#define MaxNumStaticPDBs ParamNumRandomStaticPDBs
#define MaxMemoryForStaticPDBsMB MyMin(2000, ParamMemoryLimit/2)
#define SingleStaticPDBSizeBytes \
	(NumFinalStates * BoardSize * BoardSize * BoardSize * (1 + (NumAtoms / 3)))
#define NumRandomStaticPDBs (MyMin(MaxNumStaticPDBs, \
	((MaxMemoryForStaticPDBsMB * 1000000LL) / SingleStaticPDBSizeBytes)))
#define StaticPDBSizeBytes ((1 + NumRandomStaticPDBs) * SingleStaticPDBSizeBytes)

#define Num3Groups ((NumAtoms)/3)
#if (NumAtoms % 3 == 0)
#define Num2Groups 0
#define Num1Groups 0
#elif (NumAtoms % 3 == 1)
#define Num2Groups 0
#define Num1Groups 1
#else 
#define Num2Groups 1
#define Num1Groups 0
#endif

struct StaticPDB {
	~StaticPDB() {
		if (pdb3) delete[] pdb3;
#if Num2Groups > 0
		if (pdb2) delete[] pdb2;
#elif Num1Groups > 0
		if (pdb1) delete[] pdb1;
#endif
	}

	void calculate(const State& final_state);

	void choose_groups_random();

	uchar heuristic(const State& s);

	void bfs_3(int group_index, int a, int b, int c, const State& s);

#if Num2Groups > 0
	void bfs_2(int a, int b, const State& s);
#endif 

	/* pdb3[i][a][b][c] = x means that the size-3 group with index i, when in
	 * positions a, b and c, respectively, will have heuristic = x */
	PDBDataType* pdb3 = nullptr;
	int group3[Num3Groups * 3];

	/* pdb2[a][b] = x means that the size-2 group (if it exists) will have 
	 * heuristic = x when in position a and b */
#if Num2Groups > 0
	PDBDataType* pdb2 = nullptr;
	int group2[2];
#endif 

	/* pdb2[a] = x means that the size-1 group (if it exists) will have 
	 * heuristic = x when in position a */
#if Num1Groups > 0
	PDBDataType* pdb1 = nullptr;
	int group1 = -1;
#endif

	// the atom group for a given atom. if -1, then it belongs to the 2-group
	// or 1-group
	int atom_group[NumAtoms];
};

void init_static_pbds();

void create_random_static_pdbs_greedy(
	int num_pdbs_to_create = NumRandomStaticPDBs);

void create_random_static_pdbs_random();

uchar static_pdb_heuristic(const State& s, int pdb_f_index);

#if ParamPDB == PDBStatic
extern StaticPDB pdb[NumFinalStates];
extern int random_pdb_usage[1 + MyMax(1, NumRandomStaticPDBs)];
#endif 