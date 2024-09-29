CC = g++
OPT = -O3
#OPT = -g
WARN = -Wall
CFLAGS = $(OPT) $(WARN) $(LIB)

# List all your .cpp files here (source files, excluding header files)
SIM_SRC = cache_sim.cpp cache.cpp

# List corresponding compiled object files here (.o files)
SIM_OBJ = $(SIM_SRC:.cpp=.o)
 
#################################

# default rule

all: cache_sim
	@echo "my work is done here..."


# rule for making cache_sim

cache_sim: $(SIM_OBJ)
	$(CC) -o cache_sim $(CFLAGS) $(SIM_OBJ)
	@echo "-----------DONE WITH CACHE_SIM-----------"


# generic rule for converting any .cc file to any .o file
 
.cc.o:
	$(CC) $(CFLAGS)  -c $*.cpp


# type "make clean" to remove all .o files plus the cache_sim binary

clean:
	rm -f *.o cache_sim


# type "make clobber" to remove all .o files (leaves cache_sim binary)

clobber:
	rm -f *.o
