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
#include "PDB.h"
#include "DynamicPDB.h"
#include "StaticPDB.h"
#include "OneFinalState.h"
#include "Atomix.h"
#include "Print.h"
#include <limits>
#include <algorithm>

using namespace std;

void heuristic_pdb(State& s) {
#if ParamPDB == PDBDynamic
	#if ParamHeuristic == HeuAllFinalStates 
		#if NumFinalStates > 1
	uchar pdb_heuristic = numeric_limits<uchar>::max();
	for (int i = 0; i < NumFinalStates; ++i)
		pdb_heuristic = min(pdb_heuristic, pdb[i].heuristic_matching(s));
	s.h_value = max(s.std_h_value, pdb_heuristic);
		#else // NumFinalStates == 1
	s.h_value = max(s.std_h_value, pdb[0].heuristic_matching(s));
		#endif 
	#elif ParamHeuristic == HeuOneFinalState
	s.h_value = max(s.std_h_value,
		pdb[single_final_state_index].heuristic_matching(s));
	#endif 
#elif ParamPDB == PDBMultiGoal
	s.h_value = max(s.std_h_value, pdb.heuristic_matching(s));
#elif ParamPDB == PDBStatic
	#if ParamHeuristic == HeuAllFinalStates 
		#if NumFinalStates > 1
	uchar pdb_heuristic = numeric_limits<uchar>::max();
	for (int i = 0; i < NumFinalStates; ++i)
		pdb_heuristic = min(pdb_heuristic, static_pdb_heuristic(s, i));
	s.h_value = max(s.std_h_value, pdb_heuristic);
		#else // NumFinalStates == 1
	s.h_value = max(s.std_h_value, static_pdb_heuristic(s, 0));
		#endif 
	#elif ParamHeuristic == HeuOneFinalState
	s.h_value = max(s.std_h_value, 
		static_pdb_heuristic(s, single_final_state_index));
	#endif 
#else 
	(void)s;
#endif
}