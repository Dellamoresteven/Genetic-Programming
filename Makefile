CXX?=c++
OPENCV2FLAGS=$(shell pkg-config opencv --cflags --libs)
THREADFLAGS=-lpthread
CXXFLAGS?=-std=c++20 -Wall -pedantic -Wshadow -Wstrict-aliasing -Wstrict-overflow -ggdb3

.PHONY: all msg clean fullclean

all: msg main

msg:
	@echo '--- C++20 ---'

main: GP.cpp DNA.cpp
	g++-10 ${CXXFLAGS} -O2 -o $@ $< ${OPENCV2FLAGS} ${THREADFLAGS}
