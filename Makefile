all: main test

main: 

test: request_parser_test gzip_stream_test

request_parser_test:
	g++ -std=c++17 -g tests/request_parser_test.cpp -o ./out/request_parser_test -I.

gzip_stream_test:
	g++ -std=c++17 -g tests/gzip_stream_test.cpp -o ./out/gzip_stream_test -I.