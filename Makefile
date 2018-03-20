TARGETS = glmrubysdl2

all: $(TARGETS)

SDL_PREFIX  = /mingw32
# SDL_PREFIX  = /mingw

MRB_HEAD    = ../mruby/include
MRB_LIBS    = ../mruby/build/host/lib

# SDL_CONFIG  = $(SDL_PREFIX)/bin/sdl2-config
SDL_CONFIG  = sdl2-config
CG_LIBS     = 

CROSS_COMPILE = $(SDL_PREFIX)/bin/
CC          = $(CROSS_COMPILE)gcc
CXX         = $(CROSS_COMPILE)g++

# SDLCFLAGS   = `$(SDL_CONFIG) --cflags`
SDLCFLAGS   = -I$(SDL_PREFIX)/include/SDL2 -Dmain=SDL_main
CFLAGS      = -g -Wall $(SDLCFLAGS) -I$(MRB_HEAD)
CXXFLAGS    = -g -Wall $(SDLCFLAGS) -I$(MRB_HEAD)

# SDLLIBS     = `$(SDL_CONFIG) --libs`
# SDLLIBS     = -L$(SDL_PREFIX)/lib -lmingw32 -lSDL2main -lSDL2 -mwindows
SDLLIBS     = -L$(SDL_PREFIX)/lib -lmingw32 -lSDL2main -lSDL2

# LDFLAGS     = -Wl,-rpath,$(SDL_PREFIX)/lib  -static -static-libgcc -static-libstdc++ -mwindows
LDFLAGS     = -Wl,-rpath,$(SDL_PREFIX)/lib -static -static-libgcc -static-libstdc++

# LIBS = -L$(MRB_LIBS) -lmruby -lm $(SDLLIBS) -lopengl32 -lglu32 -lm -lSDL2_image
LIBS = -L$(MRB_LIBS) -lmruby -lm $(SDLLIBS) \
  -lSDL2_image -lpng -lz -ljpeg -ltiff -lwebp -llzma \
  -ldinput8 -ldxguid -ldxerr8 -luser32 -lgdi32 -lwinmm \
  -limm32 -lole32 -loleaut32 -lshell32 -lversion -luuid \
  -lopengl32 -lglu32 -lpthread \
  -lwsock32 -lws2_32

clean:
	rm -f *.o *.a *~ $(TARGETS)

$(TARGETS): $(TARGETS).o
	$(CXX) -o $@ $^ $(LIBS) $(LDFLAGS)


