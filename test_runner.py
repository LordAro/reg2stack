#!/usr/bin/env python3
"""Runs test programs for reg2stack.

Also tests conversions
"""

import subprocess

FILEPATH = 'examples/{}.{}'

REGISTER_PROGS = ['test1', 'test2', 'bsort']
STACK_PROGS = ['loop']
CONVERSIONS = ['simple', 'loop', 'bsort', 'fib20', 'primes']

def get_prog(name, typerun, add_args=None):
    """Builds the list of commandline args for a test program
    """
    args = ['./reg2stack', '-f', '-v0']

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

for p in REGISTER_PROGS:
    prog = get_prog(p, 'r')
    print('Running', ' '.join(prog))
    ret = subprocess.run(prog, stdout=subprocess.PIPE,
                         stderr=subprocess.PIPE, check=True)
    print(ret.stdout)

for p in STACK_PROGS:
    prog = get_prog(p, 's')
    print('Running', ' '.join(prog))
    ret = subprocess.run(prog, stdout=subprocess.PIPE,
                         stderr=subprocess.PIPE, check=True)
    print(ret.stdout)

print()

for p in CONVERSIONS:
    prog = get_prog(p, 'r') # reg
    retreg = subprocess.run(prog, stdout=subprocess.PIPE,
                            stderr=subprocess.PIPE, check=True)

    prog = get_prog(p, 'c') # conv
    print('Converting', ' '.join(prog))
    retconv = subprocess.run(prog, stdout=subprocess.PIPE,
                             stderr=subprocess.PIPE)
    if retreg.stdout != retconv.stdout:
        print('Conversion result not equal!')
        print(retreg.stdout, '!=', retconv.stdout)
    else:
        print(retconv.stdout)

    # Optimise tests
    prog = get_prog(p, 'c', ['-o1'])
    retconv_o1 = subprocess.run(prog, stdout=subprocess.PIPE,
                                stderr=subprocess.PIPE)
    if retconv.stdout != retconv_o1.stdout:
        print('O1 result not equal!')
        print(retconv.stdout, '!=', retconv_o1.stdout)

    prog = get_prog(p, 'c', ['-o2'])
    retconv_o2 = subprocess.run(prog, stdout=subprocess.PIPE,
                                stderr=subprocess.PIPE)
    if retconv.stdout != retconv_o2.stdout:
        print('O2 result not equal!')
        print(retconv.stdout, '!=', retconv_o2.stdout)
