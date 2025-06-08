APPNAME = winmapviewer

# Install MinGW in Debian: apt-get install g++-mingw-w64
CC = i686-w64-mingw32-g++
RC = i686-w64-mingw32-windres

CFLAGS = -mwindows -static-libgcc -static-libstdc++
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

