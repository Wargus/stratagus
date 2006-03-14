#!/usr/bin/env python
"Output all lua functions defined in the Stratagus source code."

import os
import os, sys
from stat import *

# where to find the other stratagus tools
toolpath = os.path.dirname(sys.argv[0]) + '/'

def walktree(top, callback):
    '''recursively descend the directory tree rooted at top,
       calling the callback function for each regular file'''

    for f in os.listdir(top):
        pathname = os.path.join(top, f)
        mode = os.stat(pathname)[ST_MODE]
        if S_ISDIR(mode):
            # It's a directory, recurse into it
            walktree(pathname, callback)
        elif S_ISREG(mode):
            # It's a file, call the callback function
            callback(pathname)
        else:
            # Unknown file type, print a message
            print 'Skipping %s' % pathname

commands = []
reffiles = {}

def visitfile(file):
    #print 'visiting', file
    if file.endswith('.c'):
        for line in open(file).read().split('\n'):
          if not line.startswith('//'):
            for part in line.split('lua_register(Lua, "')[1:]:
              command = part.split('"')[0]
              commands.append(command)
              reffiles[command] = file
        
if __name__ == '__main__':
    walktree(sys.argv[1], visitfile)

    commands.sort()
    for command in commands:
       print command

