CXX = i686-elf-g++ # Use the cross compiler (i686 for x86 architecture)
CXXFLAGS = -ffreestanding -std=c++17 -O2 -Wall
LDFLAGS = -T linker.ld

SRC = kernel.cpp
OBJ = $(SRC:.cpp=.o)

all: kernel.bin

kernel.bin: $(OBJ)
	$(CXX) $(LDFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) kernel.bin
