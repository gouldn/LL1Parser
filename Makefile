CC = g++
OBJECTS = driver.o scan.o
INCLUDES = scan.h

all : driver

driver : $(OBJECTS)
	$(CC) -g -o driver $(OBJECTS)

driver.o : driver.cc scan.h
	$(CC) -c -g driver.cc

scan.o : scan.cc scan.h
	$(CC) -c -g scan.cc

clean :
	rm -f driver $(OBJECTS)
