APPNAME = winmapviewer

ARCH ?= i686

# Install MinGW in Debian: apt-get install g++-mingw-w64
CC = $(ARCH)-w64-mingw32-g++-posix
RC = $(ARCH)-w64-mingw32-windres

CFLAGS = -mwindows -static -static-libgcc -static-libstdc++ -s -Os
LFLAGS = -luser32 -lgdi32 -lcomctl32 -lwininet

SRC = $(wildcard src/*.cpp)
RC_FILE = resources/$(APPNAME).rc
BUILD_DIR = build/$(ARCH)
RES_OBJ = $(BUILD_DIR)/$(APPNAME).res.o
OBJ = $(patsubst src/%.cpp,$(BUILD_DIR)/%.o,$(SRC))

OUT = $(BUILD_DIR)/$(APPNAME).exe

all: $(OUT)

run: $(OUT)
	wine $(OUT)

$(BUILD_DIR):
	mkdir -p $@

$(BUILD_DIR)/%.o: src/%.cpp | $(BUILD_DIR)
	$(CC) -c $< -o $@ -Iinclude -Iresources

$(OUT): $(OBJ) $(RES_OBJ) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(OBJ) $(RES_OBJ) -o $(OUT) $(LFLAGS)

$(RES_OBJ): $(RC_FILE) | $(BUILD_DIR)
	$(RC) $(RC_FILE) -o $(RES_OBJ) -Iinclude -Iresources

clean:
	rm -rf build/i686 build/x86_64

