#!/usr/bin/env python
"""
Find image files with magic expansion and convert them for use with stratagus.
*-rgb.png: convert from RGB to stratagus palette
*-big.png: shrink to half size, then convert from RGB as above
TODO *.xcf: assumed to be the source of the same .png, print warning if newer

Planned: What about something like this? Would this help?
*-playercolor.png, *-unit.png: first is converted to gray, then puzzled together
                               with the right palette entries
"""

import Image, ImageFilter
import os, os.path
import sys

# where to find the other stratagus tools
toolpath = os.path.dirname(sys.argv[0]) + '/'

def convert(im):
    "returns the Image im converted to stratagus palette"
    im.convert('P', None, None, Image.ADAPTIVE, 227).save('delme.png')
    print 'You can savely ignore the following warning:'
    os.system(toolpath + 'png2stratagus delme.png delme2.png')
    res = Image.open('delme2.png')
    os.remove('delme.png')
    os.remove('delme2.png')
    return res

def mustrebuild(srcfile, dstfile):
    "check whether outfile needs to be rebuilt"
    if not os.path.isfile(dstfile):
        return 1
    return os.stat(srcfile).st_mtime > os.stat(dstfile).st_mtime
    
if __name__ == '__main__':
    if len(sys.argv) > 1:
        print argv[0], 'works with the current directory, it takes no arguments.'
        sys.exit(1)

    filelist = os.listdir('.')
    for infile in filelist:
        ext = infile[-8:]
        if ext in ['-big.png', '-rgb.png']:
            outfile = infile[:-8] + '.png'
            if mustrebuild(infile, outfile):
                print 'generating', outfile, 'from', infile
                im = Image.open(infile)
                if im.mode == 'P':
                    print 'warning:', infile, 'is already indexed, converting it to RGB first'
                    im = im.convert('RGB')
                if ext == '-big.png':
                    (w, h) = im.size
                    im = im.resize((w/2, h/2), Image.BILINEAR)
                    convert(im).save(outfile)
                elif ext == '-rgb.png':
                    convert(im).save(outfile)
    print 'done'
