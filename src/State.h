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
#include "Parameters.h"
#include "Pos.h"
#include "Definitions.h"
#include <limits>
#include <iostream>
#include <algorithm>
#include <cstdint>
#include <cstring>

/* an index should be able to index a state in the algorithm execution. so if
 * we choose a 32-bit signed integer (int), we are expecting to have less than
 * 2^31-1 states, at most. this should be a signed data type! 
 * */
typedef int Index; 

#define StateSize (NumAtoms)
struct State {
	State() {
		std::fill(&v[0], &v[0] + NumAtoms,
			std::numeric_limits<Pos>::max());
	}

	State(const State& s)
		: pq_next(s.pq_next)
		, pq_prev(s.pq_prev)
#if SaveSolutionPath
		, parent(s.parent)
#endif
		, h_value(s.h_value)
#if ParamPDB != None && \
	(ParamHeuristic == HeuOneFinalState || NumFinalStates == 1)	
		, std_h_value(s.std_h_value)
#endif 
		, g_value(s.g_value)					
		, tie_breaker(s.tie_breaker) 
#if ParamAlgorithm == AlgPEAStar
		, pea_F(s.pea_F)
#endif
	{ 
		memcpy(&v[0], &s.v[0], NumAtoms * sizeof(Pos));
	}

	State& operator=(const State& s) {
		memcpy(&v[0], &s.v[0], NumAtoms * sizeof(Pos));
		h_value = s.h_value;
		g_value = s.g_value;
		pq_next = s.pq_next;
		pq_prev = s.pq_prev;
		tie_breaker = s.tie_breaker;
#if SaveSolutionPath
		parent = s.parent;
#endif
#if ParamAlgorithm == AlgPEAStar
		pea_F = s.pea_F;
#endif
#if ParamPDB != None && \
	(ParamHeuristic == HeuOneFinalState || NumFinalStates == 1)	
		std_h_value = s.std_h_value;
#endif 
		return *this;
	}

	bool operator==(const State& s) const {
		return std::equal(&s.v[0], &s.v[0] + NumAtoms, &v[0]);
	}

	bool operator<(const State& s) const {
		return std::lexicographical_compare(
			&v[0], &v[0] + NumAtoms, &s.v[0], &s.v[0] + NumAtoms);
	}

	bool is_obstacle(Pos p) const;

	void pretty_print(std::ostream& o = std::cout) const;

	int standard_heuristic(const State& dst) const;

	int atom_standard_heuristic(const State& dst, int a) const;

	int multi_atom_matching(const State& dst, int a) const;
	
	bool contains(Pos p) const;

	size_t get_hash() const {
		size_t h = 0;
		for (int i = 0; i < StateSize; ++i) {
			h += v[i];
			h += (h << 10);
			h ^= (h >> 6);
		}

		h += (h << 3);
		h ^= (h >> 11);
		h += (h << 15);
		return h;
	}

	unsigned char f_value() const {
		return h_value + g_value;
	}

	Pos v[NumAtoms];

	Index pq_next = -1, pq_prev = -1;

#if SaveSolutionPath
	Index parent = -1;
#endif

#if ParamPDB != None && \
	(ParamHeuristic == HeuOneFinalState || NumFinalStates == 1)	
	unsigned char h_value = 0;
	unsigned char std_h_value = 0;	
#else
	union {
		unsigned char h_value = 0;
		unsigned char std_h_value;		
	};
#endif 

	unsigned char g_value = 0;		

#if ParamTieBreaking == TBFillOrder || ParamTieBreaking == TBFillOrderReverse || \
ParamTieBreaking == TBGoalCountH || ParamTieBreaking == TBHGoalCount	
	unsigned short tie_breaker = 0;
#else 
	unsigned char tie_breaker = 0;
#endif 

#if ParamAlgorithm == AlgPEAStar
	unsigned char pea_F = 0;
#endif 
};

