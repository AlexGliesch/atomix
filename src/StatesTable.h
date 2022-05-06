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
#include "TieBreaking.h"
#include "StaticPDB.h"
#include <vector>

#define HashLoadFactor 2.5
#define MemoryLimitBytes (size_t(ParamMemoryLimit * 1024LL * 1024LL))
#if ParamPDB == PDBStatic
	#define MemoryForStates (MemoryLimitBytes - StaticPDBSizeBytes)
#else 
	#define MemoryForStates MemoryLimitBytes
#endif
#define SizeState (sizeof(State))
#define MaxStates (size_t(MemoryForStates \
	/ (SizeState + sizeof(Index) * HashLoadFactor)))
#define HashTableSize (size_t(MaxStates * HashLoadFactor))

#define PqSize (GuessOnMaximumFValue*MaxTieBreakingValue) 

struct StatesTable {

	void reset(size_t num_states = MaxStates);

	void pq_push(Index s);

	void pq_update(Index s, int g);
	//void pq_update(Index s, int g, int h, int tb);

	Index pq_pop();

	int pq_index(Index s);

	Index hash_find(Index i);

	State& operator()(Index i) { return states[i]; }

	Index insert(const State& s);

	Index states_insert();

	void hash_insert(Index i);

	bool state_already_expanded(Index i);

	Index* pq;

	int smallest_pq_index = 0;

	Index* hash_table = nullptr;

	size_t hash_occupation = 0;

	State* states = nullptr;

	size_t states_top = 0;

	size_t num_states = 0;
};

extern StatesTable states_table;