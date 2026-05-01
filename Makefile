APPNAME = winmapviewer

ARCH = i686
# ARCH = x86_64

# Install MinGW in Debian: apt-get install g++-mingw-w64
CC = $(ARCH)-w64-mingw32-g++
RC = $(ARCH)-w64-mingw32-windres

CFLAGS = -mwindows -static-libgcc -static-libstdc++ -s -Os
LFLAGS = -luser32 -lgdi32 -lole32 -lcomctl32 -lwininet

SRC = $(wildcard *.cpp)
RC_FILE = $(APPNAME).rc
RES_OBJ = $(APPNAME).res.o
OBJ = $(SRC:.cpp=.o)

OUT = $(APPNAME).exe

all: $(OUT)

run: $(OUT)
	wine $(OUT)

%.o: %.cpp
	$(CC) -c $< -o $@

$(OUT): $(OBJ) $(RES_OBJ)
	$(CC) $(CFLAGS) $(OBJ) $(RES_OBJ) -o $(OUT) $(LFLAGS)

$(RES_OBJ): $(RC_FILE)
	$(RC) $(RC_FILE) -o $(RES_OBJ)

clean:
	rm -f $(OUT) $(OBJ) $(RES_OBJ)

