# Find the OS platform using the uname command.
Linux := $(findstring Linux, $(shell uname -s))
MacOS := $(findstring Darwin, $(shell uname -s))
Windows := $(findstring NT, $(shell uname -s))

# Specify what typing 'make' on its own will compile
default: test

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

display: sketch.c display.c
	clang -std=c11 -Wall -pedantic -g sketch.c display.c -lSDL2 -o sketch
	./sketch

test: sketch.c test.c
	clang -std=c11 -Wall -pedantic -g sketch.c test.c -o sketch \
	    -fsanitize=undefined -fsanitize=address 
	./sketch lines.sketch
	./sketch box.sketch
	./sketch clear.sketch
	./sketch cross.sketch
	./sketch diag.sketch
	./sketch field.sketch
	./sketch key.sketch
	./sketch lawn.sketch
	./sketch oxo.sketch
	./sketch square.sketch

fractals: fractals.c
	  clang -Dtest_fractals -std=c11 -Wall -pedantic -g fractals.c -o fractals \
	    -fsanitize=undefined -fsanitize=address
	  ./fractals
	  #./sketch out.sketch

%: %.c
	clang -Dtest_$@ -std=c11 -Wall -pedantic -g $@.c -o $@ \
	    -fsanitize=undefined -fsanitize=address

endif
