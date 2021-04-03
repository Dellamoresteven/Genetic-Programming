CXX?=c++
OPENCV2FLAGS=$(shell pkg-config opencv --cflags --libs)
CXXFLAGS?=-std=c++20 -Wall -pedantic -Wshadow -Wstrict-aliasing -Wstrict-overflow

.PHONY: all msg clean fullclean

all: msg main

msg:
	@echo '--- C++20 ---'

main: GP.cpp
	g++-10 ${CXXFLAGS} -O2 -o $@ $< ${OPENCV2FLAGS}