all: main test

main:
	g++ -std=c++17 -g -pthread -I. main.cpp -o ./out/main

test: ThreadPoolTest ZlibWrapperTest HttpRequestTest

ThreadPoolTest:
	g++ -std=c++17 -g -pthread -I. tests/ThreadPoolTest.cpp -o ./out/ThreadPoolTest

ZlibWrapperTest:
	# g++ -std=c++17 -g -pthread -I. tests/ZlibWrapperTest.cpp -o ./out/ZlibWrapperTest -lz

HttpRequestTest:
	g++ -std=c++17 -g -pthread -I. tests/HttpRequestTest.cpp -o ./out/HttpRequestTest