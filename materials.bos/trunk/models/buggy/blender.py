# This is a simple python script to create all needed animation files
# This should be used like 'blender -P blender.py'
# Sadly this needs a window popping up, adding the option '-b' to the
# command is not possible at the moment (segfault, blender bug #1655).
#
#   (c) Frank Loeffler (2004)
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

import os
import Blender

# open the requested blender file via environment variable
Blender.Load(os.environ["BLENDER_SCRIPT_FILE_TO_RENDER"])

# after the 'Blender.Load' we have to import again
import Blender
from Blender import *
from Blender.Scene import Render

# save some variables
scn = Scene.GetCurrent()
context = scn.getRenderingContext()

# render the shadow part
context.setRenderPath("shadow")
context.renderAnim()

#remove the ground (the object has to have the name 'ground')
scn.unlink(Blender.Object.Get("ground"))

# render the normal part
context.setRenderPath("normal")
context.renderAnim()

# finally quit blender
Blender.Quit()

