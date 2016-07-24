DEBUG = 0
ifeq ($(DEBUG), 1)
	CFLAGS =-DDEBUG
else
	CFLAGS =-DNDEBUG
endif

CXX = g++ $(CFLAGS)

all: Encoder

Encoder: Encoder.cpp
	$(CXX) -Wall -g Encoder.cpp -o Encoder 