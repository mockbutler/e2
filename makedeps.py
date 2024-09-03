#!/usr/bin/env python

import glob
import re

def gendeps(fn, db):
    f = open(fn)
    deps = []
    for l in f.readlines():
        m = re.match(r'\s*#include ["]([^>"]+)["]', l)
        if m:
            deps.append(m.group(1))
    f.close()
    deps.sort()
    if deps:
         db[fn] = deps

csrc = glob.glob('*.c')
deps = {}
for c in csrc:
    gendeps(c, deps)

for k, v in deps.items():
    print k + ':', ' '.join(v)


    