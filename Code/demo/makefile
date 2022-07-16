#*******************************************
# Author: Fritz Ammon
# Date: 2 September 2014
# Program: l1.c
# Description: Makefile creates program for
# Linux-only C function teseting.
# It is dependent of
# l1.o
#*******************************************

l1 : l1.o
	gcc -o l1 l1.o
l1.o : l1.c
	gcc -c l1.c 	# -c turns off linker (only creates object files).
clean :			# rm object files.
	rm *.o
