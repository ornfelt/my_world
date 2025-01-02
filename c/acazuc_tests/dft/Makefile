CXX = g++

CXXFLAGS = -Ofast -march=native -g

LDFLAGS = -lGL -lglfw

all: dft dft2 dfta

dft: dft.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

dft2: dft2.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

dfta: dfta.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	rm dft
	rm dft2
	rm dfta

.PHONY: all clean
