##   ___________		     _________		      _____  __
##   \_	  _____/______   ____   ____ \_   ___ \____________ _/ ____\/  |_
##    |    __) \_  __ \_/ __ \_/ __ \/    \  \/\_  __ \__  \   __\   __\ 
##    |     \   |  | \/\  ___/\  ___/\     \____|  | \// __ \|  |   |  |
##    \___  /   |__|    \___  >\___  >\______  /|__|  (____  /__|   |__|
##	  \/		    \/	   \/	     \/		   \/
##  ______________________                           ______________________
##			  T H E   W A R   B E G I N S
##	   FreeCraft - A free fantasy real time strategy game engine
##

# Compile commands
CC=gcc
CCLD=gcc
RM=rm -f
MAKE=make

# Prefix for 'make install'
PREFIX=/usr/local

# Use SIOD support
CCL		= -DUSE_CCL
CCLLIB		= -lm

# Video support
SDL		= -DUSE_SDL -DUSE_SDLA $(SDL_CFLAGS)
SDL_CFLAGS	= $(shell sdl-config --cflags)
SDLLIB		= $(shell sdl-config --libs)

VIDEO             = $(SDL)
VIDEOLIB	= $(SDLLIB) -ldl

# Sound support
DSOUND		= -DWITH_SOUND

# Compression support
ZDEFS		= -DUSE_ZLIB -DUSE_BZ2LIB
ZLIBS		= -lz -lbz2

XLDFLAGS	= -L/usr/X11R6/lib -L/usr/local/lib  
XIFLAGS		= -I/usr/X11R6/include -I/usr/local/include  

#####################################################################
# Don't change anything below here unless you know what you're doing!

VERSION=	'-DVERSION="1.17.2"'
PROFILE=

TOOLLIBS=$(XLDFLAGS) -lpng -lz -lm $(THREADLIB)
CLONELIBS= $(XLDFLAGS) -lpng -lz -lm \
	$(THREADLIB) $(CCLLIB) $(VIDEOLIB) $(ZLIBS) \
	$(FLACLIB) $(OGGLIB) $(MP3LIB) -lz -lm
DISTLIST=$(TOPDIR)/distlist
TAGS=$(TOPDIR)/src/tags

# Linux
EXE=
OUTFILE=$(TOPDIR)/freecraft
ARCH=linux
OE=o
OBJDIR=obj

#ARCHOBJS=stdmman.$(OE) svgalib.$(OE) unix_lib.$(OE) bitm_lnx.$(OE)
IFLAGS=	-I$(TOPDIR)/src/include $(XIFLAGS)
DFLAGS=	$(THREAD) $(CCL) $(VERSION) \
	$(VIDEO) $(ZDEFS) $(DSOUND) \
	$(DEBUG) $(SDLCD) $(LIBCDA) \
	$(FLAC) $(OGG) $(MAD) 
CFLAGS=-O2 -pipe -fsigned-char -fomit-frame-pointer -fconserve-space -fexpensive-optimizations -ffast-math  $(IFLAGS) $(DFLAGS)  -DUNIT_ON_MAP -DNEW_AI -DUSE_LIBMODPLUG -DUSE_HP_FOR_XP
CTAGSFLAGS=-i defptvS -a -f 

# Locks versions with a symbolic name
LOCKVER=	rcs -q -n$(NAME)

$(OBJDIR)/%.$(OE): %.c
	$(CC) -c $(CFLAGS) $< -o $@
	@$(AR) cru $(TOPDIR)/src/$(OBJDIR)/libclone.a $@

$(OBJDIR)/%.$(OE): %.cpp
	$(CC) -c $(CFLAGS) $< -o $@
	@$(AR) cru $(TOPDIR)/src/$(OBJDIR)/libclone.a $@

# Source code documentation
DOXYGEN=	doxygen
DOCIFY=		docify
DOCPP=		doc++

%.doc: %.c
	@$(TOPDIR)/tools/aledoc $< | $(DOCIFY) > $*-c.doc 2>/dev/null
%.doc: %.h
	@$(TOPDIR)/tools/aledoc $< | $(DOCIFY) > $*-h.doc 2>/dev/null
