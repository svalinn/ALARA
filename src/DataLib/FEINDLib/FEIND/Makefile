CXX = /opt/zeep/mpich-1.2.6_gnu/bin/mpicxx

swig: swig/FEIND.i
	swig -python -c++ swig/FEIND.i
	g++ -c -I /usr/include/python2.3/ swig/FEIND_wrap.cxx 
	mv swig/FEIND.py .

mpi:
	-rm FEIND.a
	$(CXX) *.cpp -O3 -c
	ar -cq FEIND.a *.o

all:
	-rm FEIND.a
	g++ *.cpp exception/*.cpp -c -I exception -I /home/fateneja/FEIND
	ar -cq FEIND.a *.o
	g++ -shared -o _FEIND.so *.o

