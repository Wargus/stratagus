#!/usr/bin/env python
"Update the command index in ccl-index.html."

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

head, trash, tail = open('ccl-index.html').read().split('SCRIPT')

f = open('ccl-index.html', 'w')
f.write(head + 'SCRIPT -->\n')
for command in commands:
    f.write('<dt><a href="' + reffiles[command] + '#' + command + '">' + command + '</a></dt>\n')
    f.write('<dd></dd>\n')
f.write('<!-- SCRIPT' + tail)
