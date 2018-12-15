# Find the OS platform using the uname command.
Linux := $(findstring Linux, $(shell uname -s))
MacOS := $(findstring Darwin, $(shell uname -s))
Windows := $(findstring NT, $(shell uname -s))

# Specify what typing 'make' on its own will compile
default: sketch

# For Windows, add the .exe extension
ifdef Windows

sketch: sketch.c display.c
	clang -std=c11 -Wall -pedantic -g $@.c display.c -lSDL2 -o $@.exe

test: sketch.c test.c
	clang -std=c11 -Wall -pedantic -g sketch.c test.c -o $@.exe

%: %.c
	clang -Dtest_$@ -std=c11 -Wall -pedantic -g $@.c -o $@.exe

# For Linux/MacOS, include the advanced debugging options
else

sketch: sketch.c
	clang -std=c11 -Wall -pedantic -g $@.c display.c -lSDL2 -o $@ \
	    -fsanitize=undefined -fsanitize=address

test: sketch.c test.c
	clang -std=c11 -Wall -pedantic -g sketch.c test.c -o $@ \
	    -fsanitize=undefined -fsanitize=address

%: %.c
	clang -Dtest_$@ -std=c11 -Wall -pedantic -g $@.c -o $@ \
	    -fsanitize=undefined -fsanitize=address

endif
