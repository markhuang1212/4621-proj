all: main test

main:

test: ThreadPoolTest

ThreadPoolTest:
	g++ -std=c++17 -g -pthread -I. tests/ThreadPoolTest.cpp -o ./out/ThreadPoolTest
