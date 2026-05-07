APPNAME = winmapviewer

ARCH = i686
# ARCH = x86_64

# Install MinGW in Debian: apt-get install g++-mingw-w64
CC = $(ARCH)-w64-mingw32-g++
RC = $(ARCH)-w64-mingw32-windres

CFLAGS = -mwindows -static-libgcc -static-libstdc++ -s -Os
LFLAGS = -luser32 -lgdi32 -lcomctl32 -lwininet

SRC = $(wildcard src/*.cpp)
RC_FILE = src/$(APPNAME).rc
RES_OBJ = build/$(APPNAME).res.o
OBJ = $(patsubst src/%.cpp,build/%.o,$(SRC))

OUT = build/$(APPNAME).exe

all: $(OUT)

run: $(OUT)
	wine $(OUT)

build/%.o: src/%.cpp
	$(CC) -c $< -o $@ -Iinclude

$(OUT): $(OBJ) $(RES_OBJ)
	$(CC) $(CFLAGS) $(OBJ) $(RES_OBJ) -o $(OUT) $(LFLAGS)

$(RES_OBJ): $(RC_FILE)
	$(RC) $(RC_FILE) -o $(RES_OBJ) -Iinclude -Iresources

clean:
	rm -f build/*

