CXX = g++

CXXFLAGS = -Ofast -march=native -g

LDFLAGS = -lGL -lglfw

all: modem_fm modem_am

modem_am: modem_am.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

modem_fm: modem_fm.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	rm modem_am
	rm modem_fm

.PHONY: all clean
