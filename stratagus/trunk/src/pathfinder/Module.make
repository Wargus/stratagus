SRC += src/pathfinder/astar.c \
	src/pathfinder/script_pathfinder.c \
	src/pathfinder/pathfinder.c \
	src/pathfinder/splitter.c \
	src/pathfinder/splitter_debug.c \
	src/pathfinder/splitter_lowlevel.c \
	src/pathfinder/splitter_zoneset.c

HDRS += src/pathfinder/splitter.h \
	src/pathfinder/splitter_local.h

OBJ += src/pathfinder/$(OBJDIR)/astar.o \
	src/pathfinder/$(OBJDIR)/script_pathfinder.o \
	src/pathfinder/$(OBJDIR)/pathfinder.o \
	src/pathfinder/$(OBJDIR)/splitter.o \
	src/pathfinder/$(OBJDIR)/splitter_debug.o \
	src/pathfinder/$(OBJDIR)/splitter_lowlevel.o \
	src/pathfinder/$(OBJDIR)/splitter_zoneset.o
