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
#include "Statistics.h"
#include "Parameters.h"
#include "Exceptions.h"
#include "StaticPDB.h"
#include "Print.h"
#include <iomanip>
#include <fstream>
#include <limits>
#include <algorithm>
#include <sstream>
#include <string>
#include <locale>
#include <numeric>
#include <vector>

using namespace std;

double stat_total_time = 0.0;
int stat_solution_length = numeric_limits<int>::max();
int stat_lower_bound = 0;
size_t stat_nodes_expanded = 0;
size_t stat_nodes_generated = 0;
size_t stat_num_reopened_states = 0;
size_t stat_calls_to_heuristic = 0;
int stat_nodes_generated_at_depth[GuessOnMaximumFValue];
int stat_nodes_generated_with_f_value[GuessOnMaximumFValue];
int stat_nodes_expanded_with_f_value[GuessOnMaximumFValue];
int stat_nodes_generated_with_g_value[GuessOnMaximumFValue];
int stat_nodes_expanded_with_g_value[GuessOnMaximumFValue];
int stat_nodes_generated_with_h_value[GuessOnMaximumFValue];
int stat_nodes_expanded_with_h_value[GuessOnMaximumFValue];
int stat_initial_heuristic = 0;
Timer<> stat_timer;

Timer<> stat_pdb_update_timer;
double stat_pdb_update_time = 0.0;
Timer<> stat_pdb_matching_timer;
double stat_pdb_matching_time = 0.0;

#if StatCountAvgHashProbes 
size_t stat_hash_probes = 0;
size_t stat_hash_find_calls = 0;
#endif

double stat_current_time() {
	return stat_timer.elapsed<std::ratio<1>>();
}

void stat_stop_timer() {
	stat_total_time = stat_current_time();
}

void stat_start_timer() {
	stat_total_time = 0;
	stat_timer.restart();
}

void stat_pretty_print(std::ostream& o) {
#if PrintNodesGeneratedAtDepth
	print_stats_array("Nodes generated at depth ", 
		stat_nodes_generated_at_depth, GuessOnMaximumFValue, o);
#endif 
#if StoreNodesGeneratedStats && PrintNodesGeneratedStats
	print_stats_array("Nodes generated with f-value ",
		stat_nodes_generated_with_f_value, GuessOnMaximumFValue, o);
	print_stats_array("Nodes expanded with f-value ",
		stat_nodes_expanded_with_f_value, GuessOnMaximumFValue, o);

	print_stats_array("Nodes generated with g-value ",
		stat_nodes_generated_with_g_value, GuessOnMaximumFValue, o);
	print_stats_array("Nodes expanded with g-value ",
		stat_nodes_expanded_with_g_value, GuessOnMaximumFValue, o);

	print_stats_array("Nodes generated with h-value ",
		stat_nodes_generated_with_h_value, GuessOnMaximumFValue, o);
	print_stats_array("Nodes expanded with h-value ",
		stat_nodes_expanded_with_h_value, GuessOnMaximumFValue, o);
#endif

#if PrintRandomStaticPDBUsage
	//int min_pdb_usage = *min_element(begin(random_pdb_usage), end(random_pdb_usage));
	//for (auto& i : random_pdb_usage) i -= min_pdb_usage + 1;
	print_stats_array("Usage of pdb ",
		random_pdb_usage, NumRandomStaticPDBs + 1, o);
#endif 
	print_stream(o, "Instance: ", ParamInputFile, "\n");
	print_stream(o, "Time: ", setprecision(5), fixed,
		(min(stat_total_time, (double)ParamTimeLimit)), 
		" seconds.\n");
	print_stream(o, "Nodes generated: ", 
		(stat_nodes_generated), "\n");
	print_stream(o, "Nodes expanded: ",	(stat_nodes_expanded), 
		"\n");
	print_stream(o, "Calls to heuristic: ", (stat_calls_to_heuristic),
		"\n");

#if StatCountAvgHashProbes
	print_stream(o, "Calls to hash find: ", stat_hash_find_calls, "\n");
	print_stream(o, "Average hash probes: ", setprecision(2), fixed,
		double(stat_hash_probes) / double(stat_hash_find_calls), "\n");
#endif 

	print_stream(o, "Solution length: ",
		stat_solution_length == (numeric_limits<int>::max)() ? 
		0 : stat_solution_length, "\n");
	print_stream(o, "Lower bound: ", stat_lower_bound, "\n");
	print_stream(o, "Num reopened states: ", stat_num_reopened_states, "\n");
#if ParamPDB == PDBMultiGoal or ParamPDB == PDBDynamic
	print_stream(o, "PDB update time: ", setprecision(5), fixed,
		stat_pdb_update_time / 1000.0,
		" seconds.\n");
	print_stream(o, "PDB matching time: ", setprecision(5), fixed,
		stat_pdb_matching_time / 1000.0,
		" seconds.\n");
#endif
}

void print_stats_array(std::string header_message, int* a, int size, 
		std::ostream& o) {
	string s_nodes[GuessOnMaximumFValue], s_percent[GuessOnMaximumFValue];
	int max_size_nodes = 0, max_size_percent = 0;
	int sum_all = accumulate(&a[0], &a[0] + size, 0);
	for (int i = 0; i < size; ++i) {
		if (a[i] == 0) continue;
		ostringstream ss;
		ss.imbue(locale(locale(), new comma_numpunct()));
		ss << a[i] << " ";
		s_nodes[i] = ss.str();
		max_size_nodes = max(max_size_nodes, (int)s_nodes[i].size());

		ss = ostringstream();
		ss.imbue(locale(locale(), new comma_numpunct()));
		ss << "("
			<< setprecision(2) << fixed <<
			100.0 * a[i] / (double)sum_all << "%)\n";
		s_percent[i] = ss.str();
		max_size_percent = max(max_size_percent, (int)s_percent[i].size());
	}

	for (int i = 0; i < size; ++i) {
		if (a[i] != 0) {					
			//string x = header_message 
			print_stream(o, header_message, setw(to_string(size).size()), 
				setfill(' '), i, 
				": ", setw(max_size_nodes), right, s_nodes[i],
				setw(max_size_percent), right, s_percent[i]);
		}
	}
	print_stream(o, "\n");
}

void write_output_file() {
	string path(ParamOutputFile);
	if (path.size() == 0) return;

	if (string(path, path.size() - 4, 4) != ".out")
		path += ".out";

	ofstream f(path);	

	println("Printing output to ", path);

	// remove backslashes from ifn
	string ifn(ParamInputFile);
	auto it = ifn.find_last_of("\\");
	if (it != string::npos) ifn = ifn.substr(it + 1);
	it = ifn.find_last_of("/");
	if (it != string::npos) ifn = ifn.substr(it + 1);

	println_stream(f, current_date_time_str());

	println_stream(f, ifn);
	println_stream(f, NumAtoms);
	println_stream(f, NumFinalStates);	
	println_stream(f, BoardSize);	
	println_stream(f, BoardWidth);
	println_stream(f, BoardHeight);
	println_stream(f, MoleSize);
	println_stream(f, MoleWidth);
	println_stream(f, MoleHeight);
	println_stream(f, NumFreePositions);
	println_stream(f, ParamBoard);
	println_stream(f, ParamMole);

	println_stream(f, ParamInputFile); // "Levels/atomix_01"
	println_stream(f, ParamOutputFile); // ""
	println_stream(f, ParamTimeLimit); // 30
	println_stream(f, ParamMemoryLimit); // 500
	println_stream(f, def_to_str(ParamAlgorithm)); // AlgAStar
	println_stream(f, def_to_str(ParamHeuristic)); // HeuAllFinalStates
	println_stream(f, def_to_str(ParamTieBreaking)); // None
	println_stream(f, def_to_str(ParamPDB)); // None
	println_stream(f, ParamNumRandomStaticPDBs); // 0
	println_stream(f, ParamRandomSeed); // -1

	println_stream(f, stat_total_time);
	println_stream(f, stat_solution_length == (numeric_limits<int>::max)() ?
		0 : stat_solution_length);
	println_stream(f, stat_lower_bound);
	println_stream(f, stat_nodes_expanded);
	println_stream(f, stat_nodes_generated);
	println_stream(f, stat_calls_to_heuristic);
	println_stream(f, stat_num_reopened_states);
	println_stream(f, stat_initial_heuristic);
	println_stream(f, stat_pdb_update_time / 1000.0);
	println_stream(f, stat_pdb_matching_time / 1000.0);

	vector<pair<int, int>> expanded_f, generated_f, expanded_g, generated_g, 
		expanded_h, generated_h;
	for (int i = 0; i < GuessOnMaximumFValue; ++i) {
		if (stat_nodes_expanded_with_f_value[i]) {
			expanded_f.emplace_back(i, stat_nodes_expanded_with_f_value[i]);
		}
		if (stat_nodes_generated_with_f_value[i]) {
			generated_f.emplace_back(i, stat_nodes_generated_with_f_value[i]);
		}
		if (stat_nodes_expanded_with_g_value[i]) {
			expanded_g.emplace_back(i, stat_nodes_expanded_with_g_value[i]);
		}
		if (stat_nodes_generated_with_g_value[i]) {
			generated_g.emplace_back(i, stat_nodes_generated_with_g_value[i]);
		}
		if (stat_nodes_expanded_with_h_value[i]) {
			expanded_h.emplace_back(i, stat_nodes_expanded_with_h_value[i]);
		}
		if (stat_nodes_generated_with_h_value[i]) {
			generated_h.emplace_back(i, stat_nodes_generated_with_h_value[i]);
		}
	}

	println_stream(f, generated_f.size());
	for (auto& p : generated_f)
		println_stream(f, p.first, " ", p.second);
	println_stream(f, expanded_f.size());
	for (auto& p : expanded_f)
		println_stream(f, p.first, " ", p.second);

	println_stream(f, generated_g.size());
	for (auto& p : generated_g)
		println_stream(f, p.first, " ", p.second);
	println_stream(f, expanded_g.size());
	for (auto& p : expanded_g)
		println_stream(f, p.first, " ", p.second);

	println_stream(f, generated_h.size());
	for (auto& p : generated_h)
		println_stream(f, p.first, " ", p.second);
	println_stream(f, expanded_h.size());
	for (auto& p : expanded_h)
		println_stream(f, p.first, " ", p.second);

	f.close();
}
