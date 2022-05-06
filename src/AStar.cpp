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
#include "Atomix.h"
#include "Definitions.h"
#include "OneFinalState.h"
#include "PDB.h"
#include "Parameters.h"
#include "Print.h"
#include "RandomNumberGenerator.h"
#include "Statistics.h"
#include "TieBreaking.h"
#include <algorithm>
#include <cassert>
#include <ciso646>
#include <tuple>

using namespace std;

int atom_moved = -1;
int a_star_max_moves;
State* cur_state = nullptr;
Index cur_state_index = -1;
Index a_star_solution_index = -1;

void heuristic_initial(State& s) {
#if ParamHeuristic == HeuAllFinalStates
  int best_f = 0;
  s.std_h_value = numeric_limits<uchar>::max();
  for (int f = 0; f < NumFinalStates; ++f) {
    auto h = (uchar)final_states[f].standard_heuristic(s);
    if (h < s.std_h_value) {
      s.std_h_value = h;
      best_f = f;
    }
  }
#if ParamTieBreaking != None
  tie_breaking(s, best_f);
#else
  (void)best_f;
#endif

#elif ParamHeuristic == HeuOneFinalState
  s.std_h_value = single_final_state.standard_heuristic(s);
#if ParamTieBreaking != None
  tie_breaking(s);
#endif
#endif

#if ParamPDB != None
  heuristic_pdb(s);
#endif
}

void heuristic_delta(State& s) {
  ++stat_calls_to_heuristic;
#if ParamHeuristic == HeuAllFinalStates && NumFinalStates > 1
  heuristic_initial(s);
#else
#if ParamHeuristic == HeuAllFinalStates
  static_assert(NumFinalStates == 1, "");
  auto& F = final_states[0];
#elif ParamHeuristic == HeuOneFinalState
  auto& F = single_final_state;
#endif

  s.std_h_value = cur_state->std_h_value -
                  cur_state->atom_standard_heuristic(F, atom_moved) +
                  s.atom_standard_heuristic(F, atom_moved);

#if ParamTieBreaking != None
  tie_breaking_delta(s, *cur_state, atom_moved);
#endif
#endif
  heuristic_pdb(s);
}

std::vector<State> get_solution_path(Index s) {
  std::vector<State> v;
#if SaveSolutionPath
  while (s != -1) {
    v.push_back(states_table(s));
    s = states_table(s).parent;
  }
#else
  v.push_back(states_table(s));
#endif
  return v;
}

vector<State> a_star(int max_moves) {
  auto& tb = states_table;
  a_star_max_moves = max_moves;
  a_star_solution_index = -1;
  tb.reset();

  // insert initial state
  heuristic_initial(initial_state);
  initial_state.g_value = 0;
  auto initial = tb.insert(initial_state);

  tb.pq_push(initial);

#if ParamAlgorithm == AlgPEAStar
  tb(initial).pea_F = 0;
#endif

  // main loop
  while (true) {
    cur_state_index = tb.pq_pop();
    if (cur_state_index == -1) break;

    cur_state = &tb(cur_state_index);

    if (termination_requested) return {};

    stat_lower_bound = max(stat_lower_bound, (int)cur_state->f_value());

    if (cur_state->h_value == 0) {
      return get_solution_path(cur_state_index);
    }

#if StoreNodesGeneratedStats
    ++stat_nodes_expanded_with_f_value[cur_state->f_value()];
    ++stat_nodes_expanded_with_g_value[cur_state->g_value];
    ++stat_nodes_expanded_with_h_value[cur_state->h_value];
#endif
    expand_node();

    if (a_star_solution_index != -1) {
      println("Ending because final solution has been generated.");
      return get_solution_path(a_star_solution_index);
    }
  }
  return {};
}

#if ParamAlgorithm != AlgPEAStar
void expand_node() {
  auto& tb = states_table;
  // visit all neighbours
  for (atom_moved = 0; atom_moved < NumAtoms; ++atom_moved) {
    auto atom_pos = cur_state->v[atom_moved];
    for (auto d : PosDirections) {
      auto atom_pos_moved = atom_pos;

      // atom walks
      // TODO pos_valid function doesn't need to be called here.
      while (pos_valid(atom_pos_moved + d) and
             not cur_state->is_obstacle(atom_pos_moved + d))
        atom_pos_moved += d;

      if (atom_pos == atom_pos_moved) {
        continue;
      }

      // apply move
      State& tmp = tb(0);
      tmp = *cur_state;
      tmp.v[atom_moved] = atom_pos_moved;
      tmp.g_value = cur_state->g_value + 1;

#if SaveSolutionPath
      tmp.parent = cur_state_index;
#endif

      if (atom_moved >= multi_start_index) {
        int gb = group_begin[atom_moved];
        sort(&tmp.v[0] + gb, &tmp.v[0] + gb + group_size[gb]);
      }

      Index i = tb.hash_find(0);
      if (i == -1) {
        heuristic_delta(tmp);
        if (tmp.h_value == 0) {
          a_star_max_moves = min(a_star_max_moves, (int)tmp.g_value);
          if (tmp.g_value > a_star_max_moves) continue;
#if ParamAlgorithm == AlgLayeredAStar
          // in layered A*, we can end when generating a solution node
          a_star_solution_index = 0;
          return;
#endif
        }
        if (tmp.f_value() > a_star_max_moves) continue;
        tb.pq_push(tb.insert(tmp));
      } else if (tmp.g_value < tb(i).g_value) {
        if (tmp.g_value + tb(i).h_value > a_star_max_moves) continue;
        tb.pq_update(i, tmp.g_value);

#if SaveSolutionPath
        tb(i).parent = tmp.parent;
#endif
      }
    }
  }
}
#else
void expand_node() {
  static int min_neighbours[MaxNeighbours]; /* neighbours with min f */
  static int min_neighbours_hash_indexes[MaxNeighbours];
  int neighbour_index = -1;
  int neighbours_count = 0;
  int num_min_neighbours = 0;
  int min_neighbours_f_value = numeric_limits<int>::max();
  int next_pea_F = numeric_limits<int>::max();
  auto& tb = states_table;

  for (atom_moved = 0; atom_moved < NumAtoms; ++atom_moved) {
    auto atom_pos = cur_state->v[atom_moved];
    for (auto d : PosDirections) {
      ++neighbour_index;
      auto atom_pos_moved = atom_pos;
      // TODO pos_valid function doesn't need to be called here.
      while (pos_valid(atom_pos_moved + d) and
             not cur_state->is_obstacle(atom_pos_moved + d))
        atom_pos_moved += d;

      if (atom_pos == atom_pos_moved) {
        continue;
      }

      State& tmp = tb(neighbour_index);
      tmp = *cur_state;
      tmp.v[atom_moved] = atom_pos_moved;
      tmp.pea_F = 0;

      if (atom_moved >= multi_start_index) {
        int gb = group_begin[atom_moved];
        sort(&tmp.v[0] + gb, &tmp.v[0] + gb + group_size[gb]);
      }

      Index i = tb.hash_find(neighbour_index);
      if (i != -1) {
        if (cur_state->g_value + 1 >= tb(i).g_value) {
          // re-generating already generated state. proceed only
          // if it improves
          continue;
        }
        tmp.h_value = tb(i).h_value;
        tmp.std_h_value = tb(i).std_h_value;
        tmp.tie_breaker = tb(i).tie_breaker;
      } else {
        heuristic_delta(tmp);
      }
#if SaveSolutionPath
      tmp.parent = cur_state_index;
#endif
      tmp.g_value = cur_state->g_value + 1;

      if (tmp.h_value == 0) {
        a_star_max_moves = min(a_star_max_moves, (int)tmp.g_value);
      }

      int tmp_f = tmp.f_value();
      if (tmp_f > a_star_max_moves) continue;

      if (tmp_f >= cur_state->pea_F) {
        ++neighbours_count;

        if (tmp_f <= cur_state->f_value()) {
          if (tmp_f < min_neighbours_f_value) {
            next_pea_F = min(next_pea_F, min_neighbours_f_value);
            min_neighbours_f_value = tmp_f;
            num_min_neighbours = 1;
            min_neighbours[0] = neighbour_index;
            min_neighbours_hash_indexes[0] = i;
          } else if (tmp_f == min_neighbours_f_value) {
            min_neighbours[num_min_neighbours] = neighbour_index;
            min_neighbours_hash_indexes[num_min_neighbours] = i;
            ++num_min_neighbours;
          } else {
            next_pea_F = min(next_pea_F, tmp_f);
          }
        } else {
          next_pea_F = min(next_pea_F, tmp_f);
        }
      }
    }
  }

  assert(neighbour_index == MaxNeighbours - 1);
  assert(neighbours_count >= num_min_neighbours);

  cur_state->pea_F = next_pea_F;

  if (neighbours_count > num_min_neighbours) {
    // still neighbours left, reinsert current state
    assert(next_pea_F != numeric_limits<int>::max());

    // update cur state f value
    cur_state->h_value = next_pea_F - cur_state->g_value;
    tb.pq_push(cur_state_index);
    ++stat_num_reopened_states;
  } else {
    assert(next_pea_F == numeric_limits<int>::max());
  }

  for (int j = 0; j < num_min_neighbours; ++j) {
    // insert all neighbours with f = fmin
    int h = min_neighbours[j];
    State& tmp = tb(h);

    if (tmp.h_value == 0) {
      a_star_solution_index = h;
      return;
    }

    Index i = min_neighbours_hash_indexes[j];
    if (i == -1) {
      tb.pq_push(tb.insert(tmp));
    } else { // if (tmp.g_value < tb(i).g_value) {
      assert(tmp.g_value < tb(i).g_value);
      tb.pq_update(i, tmp.g_value);
      tb(i).pea_F = 0;

#if SaveSolutionPath
      tb(i).parent = tmp.parent;
      assert(tmp.parent == cur_state_index);
#endif
    }
  }
}
#endif //  ParamAlgorithm != AlgPEAStar

vector<vector<State>> backward_bfs(State& input_state, int num_paths,
                                   int max_moves) {
  auto& tb = states_table;
  tb.reset();
  input_state.g_value = input_state.h_value = 0;
  input_state.parent = -1;
  tb.pq_push(tb.insert(input_state));
  int depth = 0;
  try {
    while (true) {
      int cur_state_index = tb.pq_pop();
      if (cur_state_index == -1) break;
      auto& cur = tb(cur_state_index);
      if (cur.g_value > depth) {
        depth = cur.g_value;
        print("depth: ", depth, "\n");
      }
      if (cur.g_value == max_moves) continue;
      if (termination_requested) break;

      for (int atom_moved = 0; atom_moved < NumAtoms; ++atom_moved) {
        auto atom_pos = cur.v[atom_moved];
        for (auto d : PosDirections) {
          assert(int(atom_pos - d) >= 0 && int(atom_pos - d) < 256);
          int atom_pos_moved = atom_pos;
          if (cur.is_obstacle(atom_pos_moved - d)) { // can have "impulse"
            while (not cur.is_obstacle(atom_pos_moved + d)) {
              assert(int(atom_pos + d) >= 0 && int(atom_pos + d) < 256);
              atom_pos_moved += d;
              State& tmp = tb(0);
              tmp = cur;
              tmp.v[atom_moved] = atom_pos_moved;
              tmp.g_value = cur.g_value + 1;
#if SaveSolutionPath
              tmp.parent = cur_state_index;
#endif
              if (atom_moved >= multi_start_index) {
                int gb = group_begin[atom_moved];
                sort(&tmp.v[0] + gb, &tmp.v[0] + gb + group_size[gb]);
              }
              Index i = tb.hash_find(0);
              if (i == -1) tb.pq_push(tb.insert(tmp));
            }
          }
        }
      }
    }
  } catch (...) {
  }
  print("broke.\n");
  vector<vector<State>> sol(num_paths);
  for (int p = 0; p < num_paths; ++p) {
    int st = random_number<int>(0, tb.states_top - 1);
    while (st != -1) {
      sol[p].push_back(tb(st));
      st = tb(st).parent;
    }
  }

  int total = 0, longest = 0;
  for (auto& path : sol) {
    total += path.size();
    longest = max(longest, (int)path.size());
  }
  print("num, total_size, longest: ", sol.size(), ", ", total, ", ", longest,
        "\n");
  return sol;
}
