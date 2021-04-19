
import sys
import os
import argparse
import subprocess
from math import ceil

import make_ms2
from modules import parent_parser

MAKE_MS2_PY_PATH = 'make_ms2'

PBS_COUNT = 0

def makePBS(mem, ppn, walltime, nThread, makeMs2_args, wd, progDir):
    global PBS_COUNT
    PBS_COUNT += 1
    pbsName = '{}/make_ms2_{}.pbs'.format(wd, PBS_COUNT)
    with open(pbsName, 'w') as outF:
        outF.write("#!/bin/tcsh\n")
        outF.write('#PBS -l mem={}gb,nodes=1:ppn={},walltime={}\n\n'.format(mem, ppn, walltime))
        outF.write('cd {}\n'.format(wd))
        outF.write('{}/{} -t {} {}\n'.format(progDir, MAKE_MS2_PY_PATH,
                                             nThread, makeMs2_args))

    return pbsName


def getPlurality(num):
    if num > 1:
        return 's'
    else: return ''


def main():
    parser = argparse.ArgumentParser(prog = 'run_make_ms2', parents=[parent_parser.PARENT_PARSER],
                                     description = 'Run rscripts/makeMS2.R and manage parallelism. '
                                                   'Program will search current working directory for .spectrum files'
                                                   ' for input.')

    parser.add_argument('-g', '--go', action = 'store_true', default = False,
                        help = 'Should jobs be submitted? If this flag is not supplied, program will be a dry run. '
                               'Required system resources will be printed but jobs will not be submitted.')

    parser.add_argument('-n', '--nJob', type=int, default = 1,
                        help='Specify number of jobs to split into.')

    parser.add_argument('-m', '--mem', default=1, type = int,
                        help = 'Amount of memory to allocate per PBS job in gb. Default is 1.')

    parser.add_argument('-p', '--ppn', default=4, type=int,
                        help='Number of processors to allocate per PBS job. Default is 4.')

    parser.add_argument('-w', '--walltime', default='12:00:00',
                        help = 'Walltime per job in the format hh:mm:ss. Default is 12:00:00.')

    args = parser.parse_args()

    progDir = os.path.dirname(sys.argv[0]).replace('python', '')
    ppn = args.ppn

    #get wd
    wd = args.dir
    if wd is None:
        wd = os.getcwd()

    spectraFiles, nFiles = make_ms2.getFileLists(args.nJob, wd)

    sys.stdout.write('{} v0.1\n'.format(parser.prog))

    #print summary of resources needed
    sys.stdout.write('\nRequested {} job{} with {} processor{} each...\n'.format(args.nJob, getPlurality(args.nJob),
                                                                              args.ppn, getPlurality(args.ppn)))
    sys.stdout.write('\t{} spectrum file{} found\n'.format(nFiles, getPlurality(nFiles)))
    if nFiles == 0:
        sys.stderr.write('No .spectra files found in {}\nExiting...\n'.format(wd))
        exit()

    filesPerJob = max([len(x) for x in spectraFiles])
    sys.stdout.write('\t{} job{} needed\n'.format(len(spectraFiles), getPlurality(len(spectraFiles))))
    sys.stdout.write('\t{} file{} per job\n'.format(filesPerJob, getPlurality(filesPerJob)))

    if filesPerJob < ppn:
        ppn = filesPerJob
    sys.stdout.write('\t{} processor{} per job\n'.format(ppn, getPlurality(ppn)))

    # get number of threads if not specified
    nThread = args.nThread
    if nThread is None:
        nThread = ppn * 2
    sys.stdout.write('\t{} thread{}\n'.format(nThread, getPlurality(nThread)))

    filesPerThread = int(ceil(float(filesPerJob) / float(nThread)))
    sys.stdout.write('\t{} file{} per thread\n\n'.format(filesPerThread, getPlurality(filesPerThread)))

    #submit jobs
    for i, item in enumerate(spectraFiles):
        makeMs2_args = '-v --mzLab {} ' \
                       '--simpleSeq {} ' \
                       '--round_to {} {}'.format(args.mzLab,
                                             args.simpleSeq,
                                             args.round_to,
                                             ' '.join(item))

        pbsName = makePBS(args.mem, args.ppn, args.walltime, nThread, makeMs2_args, wd, progDir)
        command = 'qsub {}'.format(pbsName)
        if args.verbose:
            sys.stdout.write('{}\n'.format(command))
        if args.go:
            proc = subprocess.Popen([command], cwd=wd, shell=True)
            proc.wait()

if __name__ == '__main__':
    main()
