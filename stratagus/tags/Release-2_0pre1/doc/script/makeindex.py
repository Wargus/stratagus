#!/usr/bin/env python
"Update the script index in script-index.html."

import os, string

commands = []
reffiles = {}
sep = "<!-- SCRIPT -->"

for infile in os.listdir('.'):
    if not infile.endswith('.html'): continue
    if (infile == "script-index.html"): continue
    x = open(infile).read()
    if string.find(x, sep) == -1: continue
    head, stuff, tail = x.split(sep)

    loclist = []
    for y in stuff.split("<a name=")[1:]:
	loclist.append("<a name=" + y)
	y = y.split('"')[1].split('"')[0]
	commands.append(y)
	reffiles[y] = infile
    loclist.sort()

    f = open(infile, 'w')
    f.write(head + sep + stuff.split("<a name=")[0])
    for y in loclist:
    	f.write(y)
    f.write(sep + tail)

head, trash, tail = open('script-index.html').read().split(sep)

f = open('script-index.html', 'w')
f.write(head + sep)
for command in commands:
    f.write('<dt><a href="' + reffiles[command] + '#' + command + '">' + command + '</a></dt>\n')
f.write(sep + tail)
