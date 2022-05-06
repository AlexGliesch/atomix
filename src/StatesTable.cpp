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
#include "StatesTable.h"
#include "Statistics.h"
#include "Definitions.h"
#include "Exceptions.h"
#include "Print.h"
#include "Atomix.h"
#include <cstring>
#include <algorithm>
#include <cassert>
#include <ciso646>

using namespace std;

StatesTable states_table;

void StatesTable::reset(size_t num_states) {
	this->num_states = num_states;
	if (hash_table == nullptr) {
		hash_table = new Index[HashTableSize];
	}
	memset(hash_table, -1, sizeof(Index) * HashTableSize);
	hash_occupation = 0;

	if (states == nullptr) {
		states = new State[num_states];
	}
#if ParamAlgorithm == AlgPEAStar
	states_top = 1 + MaxNeighbours;
#else
	states_top = 1;
#endif
	smallest_pq_index = numeric_limits<int>::max();

	if (pq == nullptr) {
		pq = new Index[PqSize];		
	}
	memset(pq, -1, PqSize * sizeof(Index));
}

Index StatesTable::insert(const State& s) {
	auto i = states_insert();
	states[i] = s;
	hash_insert(i);
#ifdef StoreNodesGeneratedStats
	++stat_nodes_generated_with_f_value[s.f_value()];
	++stat_nodes_generated_with_g_value[s.g_value];
	++stat_nodes_generated_with_h_value[s.h_value];
#endif 
	return i;
}

Index StatesTable::states_insert() {
	++states_top;
	if (states_top == MaxStates) {
		throw TerminationException("Maximum number of states reached.");
	}
	++stat_nodes_generated;
	return states_top - 1;
}

void StatesTable::pq_push(Index s) {	
	auto i = pq_index(s);
	/* note that this is fifo order: the newly inserted item is placed in
	* front of the queue */	
	states[s].pq_next = pq[i];
	states[s].pq_prev = -1;
	if (states[s].pq_next != -1) {
		states[states[s].pq_next].pq_prev = s;
	}
	pq[i] = s;
	smallest_pq_index = min(smallest_pq_index, i);
}

void StatesTable::pq_update(Index s, int g) {	
	auto& S = states[s];
	auto old_pq_index = pq_index(s);
	S.g_value = g;
	auto new_pq_index = pq_index(s);

	if (old_pq_index == new_pq_index) {
		return;
	}

	if (pq[old_pq_index] == s) { 
		// if s is top of pq
		assert(states[s].pq_prev == -1);
		pq[old_pq_index] = states[s].pq_next;
		if (pq[old_pq_index] != -1) {
			states[pq[old_pq_index]].pq_prev = -1;
		}
	} else {
		// is is not on top of pq
		if (states[s].pq_prev == -1) {
			++stat_num_reopened_states;
		} else {
			states[states[s].pq_prev].pq_next = states[s].pq_next;
		}
		if (states[s].pq_next != -1) {
			states[states[s].pq_next].pq_prev = states[s].pq_prev;
		}
	}

	states[s].pq_next = pq[new_pq_index];
	states[s].pq_prev = -1;
	if (states[s].pq_next != -1) {
		states[states[s].pq_next].pq_prev = s;
	}
	pq[new_pq_index] = s;
	smallest_pq_index = min(smallest_pq_index, new_pq_index);
}

Index StatesTable::pq_pop() {
	while (smallest_pq_index < PqSize) {				
		if (pq[smallest_pq_index] != -1) {			
			auto s = pq[smallest_pq_index];			
			pq[smallest_pq_index] = states[s].pq_next;
			if (states[s].pq_next != -1) {
				states[states[s].pq_next].pq_prev = -1;
			}
			states[s].pq_next = -1;			
			assert(states[s].pq_prev == -1);
			++stat_nodes_expanded;
			if (smallest_pq_index != pq_index(s)) {
				continue;
			}						
			return s;
		}
		++smallest_pq_index;
	}
	return -1;
}

Index StatesTable::pq_index(Index s) {
	assert(states[s].tie_breaker <= MaxTieBreakingValue);
#if ParamTieBreaking == None
	return states[s].f_value();
#else 
	return states[s].f_value() * (MaxTieBreakingValue + 1)
		+ MaxTieBreakingValue - states[s].tie_breaker;
#endif
}

bool StatesTable::state_already_expanded(Index i) {
	return states[i].pq_prev == -1 and states[i].pq_next == -1 and
		pq[pq_index(i)] != i;
}

void StatesTable::hash_insert(Index i) {
	size_t h = states[i].get_hash() % HashTableSize;
	while (hash_table[h] != -1) {
		if (states[hash_table[h]] == states[i]) {
			return;
		}
		h = (h + 1) % HashTableSize;
	}
	hash_table[h] = i;
	++hash_occupation;
}

Index StatesTable::hash_find(Index i) {
#if StatCountAvgHashProbes
	++stat_hash_find_calls;
	++stat_hash_probes;
#endif 
	static const size_t hts = HashTableSize;
	size_t h = states[i].get_hash() % hts;
	while (hash_table[h] != -1) {
		if (states[hash_table[h]] == states[i]) {
			return hash_table[h];
		}
		if (++h >= hts) h = 0;
		//h = (h + 1) % hts;
#if StatCountAvgHashProbes
		++stat_hash_probes;
#endif
	}
	return -1;
}
