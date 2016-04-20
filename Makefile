reconstruct3d: src/main.cpp
	g++ -o reconstruct3d src/main.cpp -Iinclude `pkg-config --cflags --libs opencv`
