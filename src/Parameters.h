#pragma once
#include "Definitions.h"
#define InstanceName "instances/adrienl_01.in"
#define NumAtoms 7
#define NumFinalStates 26
#define BoardWidth 18
#define BoardHeight 11
#define BoardSize 198
#define MoleWidth 5
#define MoleHeight 3
#define MoleSize 15
#define NumFreePositions 122
const char ParamBoard[] = "###################...#...2....#...##.#1....##.....#.##................###.....#3.#...5.###.....#....#.....###.....#..#.....###...4............##.#.....##.....#7##...#......6.#...###################";
const char ParamMole[] = "1..23.45..6..7.";

// Runtime parameters 

#define ParamInputFile "instances/adrienl_01.in"
#define ParamOutputFile ""
#define ParamTimeLimit 120
#define ParamMemoryLimit 3500
#define ParamAlgorithm AlgPEAStar
#define ParamHeuristic HeuAllFinalStates
#define ParamTieBreaking TBGoalCount
#define ParamPDB PDBStatic
#define ParamSilent false
#define ParamPrintOutputPath true
#define ParamPrintInitialHeuristic false
#define ParamNumRandomStaticPDBs 5
#define ParamRandomSeed -1
#define ParamRandomStaticPDB RandomStaticPDBGreedy