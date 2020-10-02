CFLAGS = -Wall -g -D _POSIX_C_SOURCE=200809L -Werror

all: build

build:
	# g++ -c main.cpp
	# g++ -c threads.cpp
	# g++ -o s-talk main.o threads.o instructorList.o
	g++ $(CFLAGS) main.cpp threads.cpp -lpthread -o s-talk


run: build
	./s-talk

valgrind: build
	valgrind --leak-check=full ./s-talk

clean:
	rm -f s-talk
