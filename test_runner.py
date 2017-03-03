#!/usr/bin/env python3

import subprocess

FILEPATH = 'examples/{}.{}'

REGISTER_PROGS = ['test1', 'test2']
STACK_PROGS = []
CONVERSIONS = ['test1', 'simple']

def get_prog(name, typerun, conv=False):
    args = ['./reg2stack', '-f']
    if typerun not in ['r', 's', 'c']:
        raise 'Unknown run type'
    args.append('-' + typerun)

    if typerun == 'r' or typerun == 'c':
        filename = FILEPATH.format(name, 'reg')
    elif typerun == 's':
        filename = FILEPATH.format(name, 'stack' if not conv else 'gen.stack')
    args.append('-i')
    args.append(filename)
    return args

for p in REGISTER_PROGS:
    prog = get_prog(p, 'r')
    print('Running', ' '.join(prog))
    ret = subprocess.run(prog, stdout=subprocess.PIPE,
                         stderr=subprocess.PIPE, check=True)

for p in STACK_PROGS:
    prog = get_prog(p, 's')
    print('Running', ' '.join(prog))
    ret = subprocess.run(prog, stdout=subprocess.PIPE,
                         stderr=subprocess.PIPE, check=True)

print()

for p in CONVERSIONS:
    # reg
    prog = get_prog(p, 'r')
    retreg = subprocess.run(prog, stdout=subprocess.PIPE,
                            stderr=subprocess.PIPE, check=True)

    # conv
    prog = get_prog(p, 'c')
    print('Converting', ' '.join(prog))
    retconv = subprocess.run(prog, stdout=subprocess.PIPE,
                             stderr=subprocess.PIPE)
    if retconv.returncode != 0:
        print('Error converting:')
        print(retconv.stderr)
        continue

    with open(FILEPATH.format(p, 'gen.stack'), 'wb') as f:
        f.write(retconv.stdout)
    prog = get_prog(p, 's', conv=True)
    retstack = subprocess.run(prog, stdout=subprocess.PIPE,
                              stderr=subprocess.PIPE, check=True)
    if retreg.stdout != retstack.stdout:
        print('Result not equal!')
        print(retreg.stdout, '!=', retstack.stdout)
