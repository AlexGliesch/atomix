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
#include "Timer.h"
#include "Definitions.h"
#include "Parameters.h"
#include "StaticPDB.h"
#include "Definitions.h"
#include <iostream>

extern double stat_total_time;
extern int stat_solution_length;
extern int stat_lower_bound;
extern size_t stat_nodes_expanded;
extern size_t stat_nodes_generated;
extern size_t stat_calls_to_heuristic;
extern size_t stat_num_reopened_states;
extern int stat_initial_heuristic;
extern int stat_nodes_generated_at_depth[GuessOnMaximumFValue];
extern int stat_nodes_generated_with_f_value[GuessOnMaximumFValue];
extern int stat_nodes_expanded_with_f_value[GuessOnMaximumFValue];
extern int stat_nodes_generated_with_g_value[GuessOnMaximumFValue];
extern int stat_nodes_expanded_with_g_value[GuessOnMaximumFValue];
extern int stat_nodes_generated_with_h_value[GuessOnMaximumFValue];
extern int stat_nodes_expanded_with_h_value[GuessOnMaximumFValue];
extern Timer<> stat_timer;

#define StatCountAvgHashProbes true

#if StatCountAvgHashProbes 
extern size_t stat_hash_probes;
extern size_t stat_hash_find_calls;
#endif 

double stat_current_time();

void stat_stop_timer();

void stat_start_timer();

void stat_pretty_print(std::ostream& o = std::cout);

void print_stats_array(std::string header_message, 
	int* a, int size, std::ostream& o = std::cout);

void write_output_file();

extern Timer<> stat_pdb_update_timer;
extern double stat_pdb_update_time;
extern Timer<> stat_pdb_matching_timer;
extern double stat_pdb_matching_time;

#define StoreNodesGeneratedStats true
#define PrintNodesGeneratedStats false

#define PrintNodesGeneratedAtDepth true
#define PrintRandomStaticPDBUsage true

// dont change below

#if ParamAlgorithm != AlgLayeredAStar && ParamAlgorithm != AlgIDAStar
#undef PrintNodesGeneratedAtDepth
#define PrintNodesGeneratedAtDepth false
#endif 

#if ParamPDB != PDBStatic
#undef PrintRandomStaticPDBUsage
#define PrintRandomStaticPDBUsage false
#endif 
