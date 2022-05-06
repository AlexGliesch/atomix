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
#include "AStar.h"
#include "AllFinalStates.h"
#include "Atomix.h"
#include "Exceptions.h"
#include "OneFinalState.h"
#include "Parameters.h"
#include "Print.h"
#include "RSS.h"
#include "RandomNumberGenerator.h"
#include "StaticPDB.h"
#include "Statistics.h"
#include <ciso646>
#include <fstream>
#include <iostream>
#include <sstream>
#include <thread>

using namespace std;

void count_time() {
  for (unsigned i = 0; i < ParamTimeLimit; ++i) {
    this_thread::sleep_for(chrono::seconds(1));
  }
  print("Time limit of ", ParamTimeLimit,
        "s exceeded. "
        "Terminating program.\n");
  termination_requested = true;
}

void check_memory() {
  int ms_to_sleep = 500;
  while (true) {
    this_thread::sleep_for(chrono::milliseconds(ms_to_sleep));
    current_memory = getCurrentRSS() / (1024u * 1024u);
    if (current_memory > ParamMemoryLimit * 1.05) {
      print("Memory limit of ", ParamMemoryLimit * 1.5,
            "MB exceeded. "
            "Terminating program.\n");
      termination_requested = true;
      return;
    }
  }
}

void test() {}

vector<State> run() {
#if ParamAlgorithm == AlgIDAStar
#else
#if ParamHeuristic == HeuAllFinalStates
  return all_final_states();
#elif ParamHeuristic == HeuOneFinalState
  return one_final_state();
#endif
#endif
}

void generate_nn_data() {
  int final_state_index = min(1, NumFinalStates - 1);
  auto sols = backward_bfs(final_states[final_state_index], 10000, 15);

  ostringstream ss;
  for (auto& path : sols) {
    for (int i = (int)path.size() - 2; i >= 0; --i) {
      int atom_moved = -1, dir_moved = 0;
      for (int a = 0; a < NumAtoms; ++a) {
        ss << index_to_label[a] << " " << (int)path[i].v[a] << " ";
        int pos_after = path[i].v[a], pos_before = path[i + 1].v[a];
        if (pos_after != pos_before) {
          atom_moved = a;
          if (pos_c(pos_after) > pos_c(pos_before)) dir_moved = Dir::Right;
          if (pos_c(pos_after) < pos_c(pos_before)) dir_moved = Dir::Left;
          if (pos_r(pos_after) < pos_r(pos_before)) dir_moved = Dir::Up;
          if (pos_r(pos_after) > pos_r(pos_before)) dir_moved = Dir::Down;
        }
      }
      ss << (int)atom_moved << " " << dir_moved << endl;
    }
  }
  ofstream file("nn.dat");
  file << ss.str();
}

int main(int argc, char** argv) {
  std::locale comma_locale(std::locale(), new comma_numpunct());
  std::cout.imbue(comma_locale);

  thread timer_checker_thread([&]() { count_time(); });
  timer_checker_thread.detach();

  thread memory_checker_thread([&]() { check_memory(); });
  memory_checker_thread.detach();

#if ParamPrintInitialHeuristic
  preprocess();
  println(calc_initial_heuristic());
  exit(0);
#endif

#if ParamRandomSeed >= 0
  RandomNumberGenerator<>::instance().seed(ParamRandomSeed);
#endif

  try {
    preprocess();

#if GenerateNnData
    print("Generating data to train a NN; this is for testing. See "
          "Definitions.h to disable it.\n");
    generate_nn_data();
    exit(EXIT_SUCCESS);
#endif

    if (argc == 2 and string(argv[1]) == "-t") {
      test();
      exit(EXIT_SUCCESS);
    }
    stat_initial_heuristic = calc_initial_heuristic();
    println("Running...");
    auto x = run();
    stat_solution_length = x.size() ? x[0].f_value() : 0;
    stat_lower_bound = x.size() ? stat_solution_length : stat_lower_bound;
    if (stat_solution_length > 0) {
      println("\nSolution found!");
      x[0].pretty_print();
      println("");

      if (ParamPrintOutputPath) {
        println("Solution path:");
        for (int i = (int)x.size() - 1; i >= 0; --i) {
          // println("begin_path ", x.size() - i);
          x[i].pretty_print();
          // println("end_path ", x.size() - i);
        }
      }
    }
  } catch (std::bad_alloc& e) {
    println("Bad alloc: ", e.what());
    exit(EXIT_FAILURE);
  } catch (TerminationException& e) {
    println("Termination Exception: ", e.what(), "\n");
    stat_solution_length = 0;
  } catch (std::exception& e) {
    print("Error, exception thrown: ");
    print(e.what());
    exit(EXIT_FAILURE);
  }
  stat_stop_timer();
  write_output_file();
  stat_pretty_print();
  println("\n");
}
