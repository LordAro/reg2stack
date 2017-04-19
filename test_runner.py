#!/usr/bin/env python3
"""Runs test programs for reg2stack.

Also tests conversions
"""

import re
import subprocess

FILEPATH = 'examples/{}.{}'

REGISTER_PROGS = ['test1', 'test2', 'bsort']
STACK_PROGS = ['loop']
CONVERSIONS = ['simple', 'loop', 'redundant', 'bsort', 'fib20', 'primes', 'tri100']

def get_prog(name, typerun, add_args=None, verbose=0):
    """Builds the list of commandline args for a test program
    """
    args = ['./reg2stack', '-f']
    args.append('-v' + str(verbose))

    if add_args is not None:
        args.extend(add_args)

    if typerun not in ['r', 's', 'c']:
        raise 'Unknown run type'
    args.append('-' + typerun)

    if typerun == 'r' or typerun == 'c':
        filename = FILEPATH.format(name, 'reg')
    elif typerun == 's':
        filename = FILEPATH.format(name, 'stack')
    args.append(filename)
    return args


def run_prog(prog):
    """Runs the program
    """
    print('Running', ' '.join(prog))
    return subprocess.run(prog, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                          check=True)


def print_metric(output):
    """Takes output from a -v2 conversion run and notes some metrics about it
    """
    output = output.decode("utf-8")
    traces = re.findall("^\[2\] \t(.*)$", output, flags=re.MULTILINE)
    traces_loadstore = re.findall("^\[2\] \t(LOAD|STORE)$", output,
                                  flags=re.MULTILINE)

    print("Trace count:", len(traces))
    print("Trace LOAD/STORE count:", len(traces_loadstore))

for p in REGISTER_PROGS:
    ret = run_prog(get_prog(p, 'r'))
    print(ret.stdout)

print()

for p in STACK_PROGS:
    ret = run_prog(get_prog(p, 's'))
    print(ret.stdout)

print()

for p in CONVERSIONS:
    retreg = run_prog(get_prog(p, 'r')) # reg

    retconv = run_prog(get_prog(p, 'c'))
    if retreg.stdout != retconv.stdout:
        print('Conversion result not equal!')
        print(retreg.stdout, '!=', retconv.stdout)
        break
    else:
        print(retconv.stdout)

    # Optimise tests
    retconv_o1 = run_prog(get_prog(p, 'c', ['-o1']))
    if retconv.stdout != retconv_o1.stdout:
        print('O1 result not equal!')
        print(retconv.stdout, '!=', retconv_o1.stdout)
        break

    retconv_o2 = run_prog(get_prog(p, 'c', ['-o2']))
    if retconv.stdout != retconv_o2.stdout:
        print('O2 result not equal!')
        print(retconv.stdout, '!=', retconv_o2.stdout)
        break

    # Call metric tests
    retconv_o0_v2 = run_prog(get_prog(p, 'c', ['-o0'], verbose=2))
    print_metric(retconv_o0_v2.stdout)
    retconv_o1_v2 = run_prog(get_prog(p, 'c', ['-o1'], verbose=2))
    print_metric(retconv_o1_v2.stdout)
    retconv_o2_v2 = run_prog(get_prog(p, 'c', ['-o2'], verbose=2))
    print_metric(retconv_o2_v2.stdout)
