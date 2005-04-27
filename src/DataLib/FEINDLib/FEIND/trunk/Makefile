CXX = /opt/zeep/mpich-1.2.6_gnu/bin/mpicxx
mpi:
	-rm FEIND.a
	$(CXX) *.cpp -O3 -c
	ar -cq FEIND.a *.o

all:
	-rm FEIND.a
	g++ *.cpp -O3 -c
	ar -cq FEIND.a *.o
