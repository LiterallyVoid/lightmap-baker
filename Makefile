OBJ = src/main.o src/map.o src/texture.o src/compute.o
EXE = lightmap

CXXFLAGS = -g -Og
LDFLAGS = -lX11 -lglfw -lGL -lm -lpng -lGLU -lnoise -lGLEW

$(EXE): $(OBJ)
	$(CXX) $^ -o $@ $(LDFLAGS)
