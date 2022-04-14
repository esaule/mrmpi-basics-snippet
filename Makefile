CXX=mpicxx
CC=mpicxx
LD=mpicxx
CXXFLAGS=-O2 -std=c++14 -I /usr/include/mrmpi
LDLIBS=-lMapReduceMPI

pi:


clean:
	-rm *.o
	-rm pi

distclean:
	-rm *.sh.*

.PHONY: test plot
