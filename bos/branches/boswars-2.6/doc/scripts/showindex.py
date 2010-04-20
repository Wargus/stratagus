#!/usr/bin/env python
""" Print a line for each function in index.html.
  Usefull to check if all functions are documented.
"""

import os

commands = []
reffiles = {}

for infile in os.listdir('.'):
    if not infile.endswith('.html'): continue
    for part in open(infile).read().split('<a name="')[1:]:
        command = part.split('"')[0]
        commands.append(command)
        reffiles[command] = infile

commands.sort()

for command in commands:
     print command
