CC=g++
CFLAGS=-g -Wall -W -pedantic -ansi -std=c++11

INCLUDEDIR=include
LDFLAGS=-I$(INCLUDEDIR) -I3rd_party/Fast-Quadric-Mesh-Simplification `pkg-config --cflags --libs opencv`

SOURCEDIR=src
_SOURCES=main.cpp depth_map.cpp mesh.cpp disparity_map.cpp camera.cpp image_pair.cpp
SOURCES=$(patsubst %,$(SOURCEDIR)/%,$(_SOURCES))

EXECUTABLE=reconstruct3d

all: $(EXECUTABLE)

$(EXECUTABLE): $(SOURCES)
	$(CC) -o $@ $(SOURCES) $(CFLAGS) $(LDFLAGS)

.PHONY: clean

clean:
	rm -f $(SOURCEDIR)/*.o $(EXECUTABLE)
