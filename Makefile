CC =  gcc # Set the compiler
#L_FLAGS = -O3 # comment to get proper prints in gdb no optimized variables
C_FLAGS = -g -lm # uncomment to get debug symbols

all: cf bSort
.PHONY: all

# Project compilation
cf: count_stuff.c
	$(CC) $<  -o $@ $(C_FLAGS) $(L_FLAGS)

bSort: bitonic_sort.c
	$(CC) $<  -o $@ $(C_FLAGS) $(L_FLAGS)

.PHONY: clean

clean:
	rm -f *.c~
	rm -f *.o
	rm cf

