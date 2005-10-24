##       _________ __                 __                               
##      /   _____//  |_____________ _/  |______     ____  __ __  ______
##      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
##      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ \ 
##     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
##             \/                  \/          \//_____/            \/ 
##  ______________________                           ______________________
##			  T H E   W A R   B E G I N S
##	   Stratagus - A free fantasy real time strategy game engine
##
##	Module.make	-	Module Makefile (included from Makefile).
##
##	(c) Copyright 2004 by The Stratagus Team
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
font.cpp \
graphics.cpp \
gui.cpp \
guichan.cpp \
image.cpp \
imagefont.cpp \
key.cpp \
keyinput.cpp \
mouseinput.cpp \
rectangle.cpp \
sdl/sdl.cpp \
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

HDRS += \
include/guichan/actionlistener.hpp \
include/guichan/allegro.hpp \
include/guichan/basiccontainer.hpp \
include/guichan/cliprectangle.hpp \
include/guichan/color.hpp \
include/guichan/defaultfont.hpp \
include/guichan/exception.hpp \
include/guichan/focushandler.hpp \
include/guichan/font.hpp \
include/guichan/graphics.hpp \
include/guichan/gui.hpp \
include/guichan/image.hpp \
include/guichan/imagefont.hpp \
include/guichan/imageloader.hpp \
include/guichan/input.hpp \
include/guichan/key.hpp \
include/guichan/keyinput.hpp \
include/guichan/keylistener.hpp \
include/guichan/listmodel.hpp \
include/guichan/mouseinput.hpp \
include/guichan/mouselistener.hpp \
include/guichan/opengl/openglgraphics.hpp \
include/guichan/opengl/openglimageloader.hpp \
include/guichan/opengl.hpp \
include/guichan/platform.hpp \
include/guichan/rectangle.hpp \
include/guichan/sdl/sdlgraphics.hpp \
include/guichan/sdl/sdlimageloader.hpp \
include/guichan/sdl/sdlinput.hpp \
include/guichan/sdl/sdlpixel.hpp \
include/guichan/sdl.hpp \
include/guichan/widget.hpp \
include/guichan/widgets/button.hpp \
include/guichan/widgets/checkbox.hpp \
include/guichan/widgets/container.hpp \
include/guichan/widgets/dropdown.hpp \
include/guichan/widgets/icon.hpp \
include/guichan/widgets/label.hpp \
include/guichan/widgets/listbox.hpp \
include/guichan/widgets/radiobutton.hpp \
include/guichan/widgets/scrollarea.hpp \
include/guichan/widgets/slider.hpp \
include/guichan/widgets/textbox.hpp \
include/guichan/widgets/textfield.hpp \
include/guichan/widgets/window.hpp \
include/guichan/x.hpp \
include/guichan.hpp
