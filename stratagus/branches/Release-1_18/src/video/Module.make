SRC += src/video/X11.c src/video/cursor.c src/video/deco.c src/video/font.c src/video/graphic.c src/video/linedraw.c src/video/new_X11.c src/video/png.c src/video/sdl.c src/video/sprite.c src/video/svgalib.c src/video/sweepline.c src/video/video.c src/video/wince.c 
HDRS += src/video/intern_video.h src/video/sweepline.h
OBJ += src/video/$(OBJDIR)/X11.o src/video/$(OBJDIR)/cursor.o src/video/$(OBJDIR)/deco.o src/video/$(OBJDIR)/font.o src/video/$(OBJDIR)/graphic.o src/video/$(OBJDIR)/linedraw.o src/video/$(OBJDIR)/new_X11.o src/video/$(OBJDIR)/png.o src/video/$(OBJDIR)/sdl.o src/video/$(OBJDIR)/sprite.o src/video/$(OBJDIR)/svgalib.o src/video/$(OBJDIR)/sweepline.o src/video/$(OBJDIR)/video.o src/video/$(OBJDIR)/wince.o 
MISC += src/video/_clip_rectangle
