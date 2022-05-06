#
# Solving Atomix with pattern databases
# Copyright (c) 2016 Alex Gliesch, Marcus Ritt
#
# Permission is hereby granted, free of charge, to any person (the "Person")
# obtaining a copy of this software and associated documentation files (the
# "Software"), to deal in the Software, including the rights to use, copy, modify,
# merge, publish, distribute the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# 1. The above copyright notice and this permission notice shall be included in
#    all copies or substantial portions of the Software.
# 2. Under no circumstances shall the Person be permitted, allowed or authorized
#    to commercially exploit the Software.
# 3. Changes made to the original Software shall be labeled, demarcated or
#    otherwise identified and attributed to the Person.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
# COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
# IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#


import subprocess, glob, sys, socket, argparse, os
from time import gmtime, strftime

def cmd(c, **kwargs):
	try:	
		x = subprocess.check_output(c, shell=True, universal_newlines=True, 
			**kwargs)
	except Exception as e:
		x = e.output
		print x
		sys.exit()
	return str(x)

if len(sys.argv) == 2 and sys.argv[1] == 'clean':
	cmd('make clean', cwd='src')
	sys.exit()

parser = argparse.ArgumentParser(description='Compute exact and heuristic \
	solutions for Atomix.', add_help=True)

args = {}

defineConvert = {
	'a': 'AlgAStar',
	'astar': 'AlgAStar',
	'ida' : 'AlgIDAStar',
	'idastar': 'AlgIDAStar',
	'pea': 'AlgPEAStar',
	'peastar': 'AlgPEAStar',
	'layer': 'AlgLayeredAStar',
	'layered': 'AlgLayeredAStar',
	'layeredastar': 'AlgLayeredAStar',
	'afs': 'HeuAllFinalStates', 
	'allfinalstates': 'HeuAllFinalStates',
	'ofs': 'HeuOneFinalState',
	'onefinalstate': 'HeuOneFinalState',
	'none': 'None',
	'h': 'TBH',
	'gc': 'TBGoalCount',
	'gch': 'TBGoalCountH',
	'goalcounth': 'TBGoalCountH',
	'hgc': 'TBHGoalCount',
	'hgoalcount': 'TBHGoalCount',	
	'goalcount': 'TBGoalCount',
	'fo': 'TBFillOrder',
	'fillorder': 'TBFillOrder',
	#'random': 'TBRandom',
	'nrp': 'TBNumberRealizablePaths',
	'numrealizablepaths': 'TBNumberRealizablePaths',
	'static': 'PDBStatic', 
	'dynamic': 'PDBDynamic',
	'multigoal': 'PDBMultiGoal',
	'random': 'RandomStaticPDBRandom',
	'greedy': 'RandomStaticPDBGreedy'
	}


# =============================================================================
# Write parameters 
# =============================================================================

def writeParameters(inputPath, outputPath):
	with open(inputPath, 'r') as fin:
		lines = fin.readlines()
		lines = [x[:-1] for x in lines] # remove \n		
		instanceName = lines[0]
		numAtoms = lines[1]

		x = lines[2].split(' ')
		boardWidth, boardHeight = x[0], x[1]
		boardSize = str(int(boardWidth) * int(boardHeight))
		board = ''
		for i in xrange(int(boardHeight)):
			board += lines[3 + i]
		board = board.replace('\r', '')

		x = lines[3 + int(boardHeight)]
		moleWidth, moleHeight = x.split(' ')[0], x.split(' ')[1]
		mole = ''
		for i in xrange(int(moleHeight)):
			mole += lines[4 + int(boardHeight) + i]
		mole = mole.replace('\r', '')
		
		numFinalStates = lines[4 + int(boardHeight) + int(moleHeight)]
		numFreePositions = lines[5 + int(boardHeight) + int(moleHeight)]
			
		with open('src/Parameters.h', 'w') as fout:
			fout.write('#pragma once\n#include \"Definitions.h\"\n')
			fout.write('#define InstanceName \"' + 
			  inputPath.replace('Levels/', '') + '\"\n')
			fout.write('#define NumAtoms ' + numAtoms + '\n')
			fout.write('#define NumFinalStates ' + numFinalStates + '\n')
			fout.write('#define BoardWidth ' + boardWidth + '\n')
			fout.write('#define BoardHeight ' + boardHeight + '\n')
			fout.write('#define BoardSize ' + boardSize + '\n')
			fout.write('#define MoleWidth ' + moleWidth + '\n')
			fout.write('#define MoleHeight ' + moleHeight + '\n')
			fout.write('#define MoleSize ' + 
				str(int(moleWidth) * int(moleHeight)) + '\n')
			fout.write('#define NumFreePositions ' + numFreePositions)
			fout.write('\n')
			fout.write('const char ParamBoard[] = \"' + board + '\";\n')
			fout.write('const char ParamMole[] = \"' + mole + '\";\n')
			fout.write('\n// Runtime parameters \n\n')
			fout.write('#define ParamInputFile \"' + 
			  inputPath.replace('Levels/', '') + '\"\n')
			fout.write('#define ParamOutputFile \"' + outputPath +'\"\n')
			fout.write('#define ParamTimeLimit ' + str(args['time']) + '\n')
			fout.write('#define ParamMemoryLimit ' + str(args['memory']) + '\n')
			fout.write('#define ParamAlgorithm ' + defineConvert[args['alg']] 
				+ '\n')
			fout.write('#define ParamHeuristic ' + defineConvert[args['heu']]
				+ '\n')
			fout.write('#define ParamTieBreaking ' + defineConvert[args['tb']] 
				+ '\n')
			fout.write('#define ParamPDB ' + defineConvert[args['pdb']] + '\n')
			fout.write('#define ParamSilent ' 
				+ str(args['silent']).lower() + '\n')
			fout.write('#define ParamPrintOutputPath ' 
				+ str(args['path']).lower() + '\n')
			fout.write('#define ParamPrintInitialHeuristic ' 
				+ str(args['initial']).lower() + '\n')
			fout.write('#define ParamNumRandomStaticPDBs '
				+ str(args['numstatic']) + '\n')
			fout.write('#define ParamRandomSeed ' + str(args['seed']) + '\n')
			fout.write('#define ParamRandomStaticPDB ' + 
			  defineConvert[args['randomstatic']])

# =============================================================================
# Parameter options 
# =============================================================================

algorithmChoices = ['astar', 'idastar', 'layeredastar', 'peastar', 'pea', 
					'layered', 'layer', 'a', 'ida']
heuristicChoices = ['onefinalstate', 'ofs', 'allfinalstates', 'afs']
tbChoices = ['fillorder', 'goalcount', 'numrealizablepaths', 'fo', 
	'gc', 'nrp', 'h', 'gch', 'hgc', 'goalcounth', 'hgoalcount', 'none']
pdbChoices = ['static', 'dynamic', 'multigoal', 'none']

# =============================================================================
# Setup argparse parameters 
# =============================================================================
parser.add_argument('-m', '--memory', 
	required = False, 
	type = int, 
	default = 500,
	help = 'memory limit (MB)')

parser.add_argument('-t', '--time', 
	required = False, 
	type = int, 
	default = 30,
	help = 'time limit (seconds)')

parser.add_argument('-i', '--in', 
	required = True, 
	type = str, 
	help = 'input file')

parser.add_argument('-o', '--out', 
	required = False, 
	type = str, 
	default = '',
	help = 'output file')

parser.add_argument('--overrideoutfiles', 	
	action='store_true',
	help = 'will override the existent output files')

parser.add_argument('--alg',
	required = False,
	type = str, 	
	choices = algorithmChoices,
	default = 'peastar',
	help = 'algorithm to run')

parser.add_argument('--heu', 
	required = False,
	type = str, 	
	choices = heuristicChoices,
	default = 'afs',
	help = 'heuristic to run')

parser.add_argument('--tb', 
	required = False,
	type = str,
	choices = tbChoices,
	default = 'gc',
	help = 'tie breaking rule')

parser.add_argument('--pdb', 
	required = False,
	type = str,
	choices = pdbChoices,
	default = 'static',
	help = 'which pattern database to use')

parser.add_argument('--seed', 
	required = False,
	type = int,
	default = -1,
	help = ('seed to use for random number generation; if smaller than 0 or ' + 
	'not specified, a random seed will be used'))

parser.add_argument('--numstatic', 
	required = False,
	type = int,
	default = 5,
	help = 'number of random static PDBs')

parser.add_argument('--randomstatic',
	required = False,
	type = str, 
	choices = ['greedy', 'random'],
	default = 'greedy',
	help = 'randomization method for static PDB. \'greedy\': choose ' + 
	'random partitions that minimize inter-group distance; \'random\': one ' + 
	'greedy partition, and the rest are fully random')

parser.add_argument('--silent',
	help = 'silent option, program will not print to stdout',
	action = 'store_true')

parser.add_argument('--path',
	help = 'if a solution path is found, it will be output',
	action = 'store_true')

parser.add_argument('--initial',
	help = 'the program will only print the initial heuristic value, and exit',
	action = 'store_true')

parser.add_argument('--build', 
	help = 'if this option is set, the script will only compile Atomix, and ' + 
		'not run it', 
	action = 'store_true')

parser.add_argument('--nproc', 
	required = False,
	type = int, 
	default = 0,
	help = 'number of processors with which to build')

parser.add_argument('--ndebug', 
	action = 'store_true',
	help = 'compile with #define NDEBUG')
	

# =============================================================================
# main 
# =============================================================================

args = vars(parser.parse_args())

inputInstances = glob.glob(args['in'])
inputInstances = [i for i in inputInstances if len(i) > 0 and i.endswith('.in')]

if len([i for i in inputInstances if not os.path.isfile(i)]):
	print ('Error: an input file does not exist, or is not a valid file.')
	sys.exit()

if (defineConvert[args['heu']] == 'HeuOneFinalState' and 
		(defineConvert[args['alg']] == 'AlgAStar' or 
		 defineConvert[args['alg']] == 'AlgPEAStar')):
	print ('Error: One Final State heuristic only admits IDA* or Layered A* as '
		+ 'algorithm choices')
	sys.exit()

if len(inputInstances) == 0:
	print 'No input instances found.'
	sys.exit()

for i in inputInstances:
	if os.path.isfile(args['out']):
		outputInstance = str(args['out'])
	elif os.path.isdir(args['out']):
		outputInstance = args['out'] + ('' 
			if args['out'].endswith('/') else '/')
		outputInstance += i.split('/')[-1].replace('.in', '.out')
		if args['overrideoutfiles']:
			if os.path.exists(outputInstance):
				print cmd('rm ' + outputInstance)
		elif os.path.isfile(outputInstance):
			print ('Ignoring input file ' + i + ' because output file ' + 
					'already exists.\n')
			continue
		outputInstance = '../' + outputInstance
	else:
		outputInstance = ''

	writeParameters(i, outputInstance)
	nproc = (str(args['nproc']) if args['nproc'] > 0 
		  else str(int(cmd('nproc')) / 2))
	makeCommand = 'make -j ' + nproc + (' CFLAGS=-DNDEBUG' 
		if args['ndebug'] else '')
	print makeCommand
	res = cmd(makeCommand, cwd='src')
	if 'failed' in res.lower() in res.lower():
		sys.exit()
	print ' '.join(sys.argv) + '\n'

	if not args['build']:
		sys.stdout.write(cmd('./atomix', cwd='src'))
	else:
		print 'Successful build'

