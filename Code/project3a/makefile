# Fritz Ammon
# Makefile for project3a

p3a : procTypeImp.o linQueueTypeImp.o schedTypeImp.o project3a.o
	g++ procTypeImp.o linQueueTypeImp.o schedTypeImp.o project3a.o -o p3a
procTypeImp.o : procTypeImp.cpp
	g++ -c procTypeImp.cpp
linQueueTypeImp.o : linQueueTypeImp.cpp
	g++ -c linQueueTypeImp.cpp
schedTypeImp.o : schedTypeImp.cpp
	g++ -c schedTypeImp.cpp
project3a.o : project3a.cpp
	g++ -c project3a.cpp
clean :
	rm *.o