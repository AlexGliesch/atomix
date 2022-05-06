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
#define MyMin(a, b) ((a) < (b) ? (a) : (b))
#define MyMax(a, b) ((a) > (b) ? (a) : (b))

#define AlgAStar 1
#define AlgIDAStar 2
#define AlgLayeredAStar 13
#define AlgPEAStar 14
#define HeuOneFinalState 3
#define HeuAllFinalStates 4
#define TBGoalCount 5
#define TBFillOrder 6
#define TBFillOrderReverse 12
#define TBRandom 15
#define TBNumberRealizablePaths 7
#define TBH 16
#define TBHGoalCount 17
#define TBGoalCountH 18
#define PDBDynamic 8
#define PDBStatic 9
#define PDBMultiGoal 10
#define RandomStaticPDBRandom 21
#define RandomStaticPDBGreedy 22
#define None 11

inline const char* def_to_str(int def) {
	switch (def) {
	case AlgAStar: return "A*";
	case AlgIDAStar: return "IDA*";
	case AlgLayeredAStar: return "Layered A*";
	case AlgPEAStar: return "Partial Expansion A*";
	case HeuOneFinalState: return "One Final State";
	case HeuAllFinalStates: return "All Final States";
	case TBGoalCount: return "Goal Count";
	case TBFillOrder: return "Fill Order";
	case TBFillOrderReverse: return "Reverse Fill Order";
	case TBRandom: return "Random";
	case TBNumberRealizablePaths: return "Number of Realizable Paths";
	case TBH: return "h";
	case TBHGoalCount: return "h - Goal Count";
	case TBGoalCountH: return "Goal Count - h";
	case PDBDynamic: return "Dynamic";
	case PDBStatic: return "Static";
	case PDBMultiGoal: return "Multi-goal";
	case RandomStaticPDBRandom: return "Random Static PDB Random";
	case RandomStaticPDBGreedy: return "Random Static PDB Greedy";
	case None: return "None";
	default: return "None";
	}
}

typedef unsigned char uchar;

#define GuessOnMaximumFValue 150 /* it's a guess of the upper bound on the f-value of any instance */

#define MaxNeighbours (4*NumAtoms)
#define SaveSolutionPath true

#define GenerateNnData false

//#define NDEBUG