##     ____                _       __               
##    / __ )____  _____   | |     / /___ ___________
##   / __  / __ \/ ___/   | | /| / / __ `/ ___/ ___/
##  / /_/ / /_/ (__  )    | |/ |/ / /_/ / /  (__  ) 
## /_____/\____/____/     |__/|__/\__,_/_/  /____/  
##                                              
##       A futuristic real-time strategy game.
##          This file is part of Bos Wars.
##
##      Module.make - Module Makefile (included from Makefile).
##
##      (c) Copyright 2004-2006 by The Stratagus Team
##
##      This program is free software; you can redistribute it and/or modify
##      it under the terms of the GNU General Public License as published by
##      the Free Software Foundation; only version 2 of the License.
##
##      This program is distributed in the hope that it will be useful,
##      but WITHOUT ANY WARRANTY; without even the implied warranty of
##      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##      GNU General Public License for more details.
##
##      You should have received a copy of the GNU General Public License
##      along with this program; if not, write to the Free Software
##      Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

MODULE = src/guichan
MSRC =   \
cliprectangle.cpp \
color.cpp \
defaultfont.cpp \
exception.cpp \
focushandler.cpp \
gfont.cpp \
graphics.cpp \
gui.cpp \
guichan.cpp \
image.cpp \
imagefont.cpp \
key.cpp \
keyinput.cpp \
mouseinput.cpp \
rectangle.cpp \
sdl/gsdl.cpp \
sdl/sdlgraphics.cpp \
sdl/sdlimageloader.cpp \
sdl/sdlinput.cpp \
widget.cpp \
widgets/button.cpp \
widgets/checkbox.cpp \
widgets/container.cpp \
widgets/dropdown.cpp \
widgets/icon.cpp \
widgets/label.cpp \
widgets/listbox.cpp \
widgets/radiobutton.cpp \
widgets/scrollarea.cpp \
widgets/slider.cpp \
widgets/textbox.cpp \
widgets/textfield.cpp \
widgets/window.cpp \

SRC +=   $(addprefix $(MODULE)/,$(MSRC))

MHDRS = \
include/guichan/actionlistener.h \
include/guichan/allegro.h \
include/guichan/basiccontainer.h \
include/guichan/cliprectangle.h \
include/guichan/color.h \
include/guichan/defaultfont.h \
include/guichan/exception.h \
include/guichan/focushandler.h \
include/guichan/font.h \
include/guichan/graphics.h \
include/guichan/gsdl.h \
include/guichan/gui.h \
include/guichan/image.h \
include/guichan/imagefont.h \
include/guichan/imageloader.h \
include/guichan/input.h \
include/guichan/key.h \
include/guichan/keyinput.h \
include/guichan/keylistener.h \
include/guichan/listmodel.h \
include/guichan/mouseinput.h \
include/guichan/mouselistener.h \
include/guichan/platform.h \
include/guichan/rectangle.h \
include/guichan/sdl/sdlgraphics.h \
include/guichan/sdl/sdlimageloader.h \
include/guichan/sdl/sdlinput.h \
include/guichan/sdl/sdlpixel.h \
include/guichan/widget.h \
include/guichan/widgets/button.h \
include/guichan/widgets/checkbox.h \
include/guichan/widgets/container.h \
include/guichan/widgets/dropdown.h \
include/guichan/widgets/icon.h \
include/guichan/widgets/label.h \
include/guichan/widgets/listbox.h \
include/guichan/widgets/radiobutton.h \
include/guichan/widgets/scrollarea.h \
include/guichan/widgets/slider.h \
include/guichan/widgets/textbox.h \
include/guichan/widgets/textfield.h \
include/guichan/widgets/window.h \
include/guichan/x.h \
include/guichan.h

HDRS +=  $(addprefix $(MODULE)/,$(MHDRS))
