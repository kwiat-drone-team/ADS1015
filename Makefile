CC=g++
CFLAGS=-I./include
SRC=./src/
OBJ=./bin/
ALL_OBJ = $(wildcard ./bin/*.o)

exec: PiezoControl.o PiezoController.o PSDInterface.o
	$(CC) -o exec $(SRC)main.cpp $(ALL_OBJ) $(CFLAGS) -pthread

PiezoControl.o: binfolder PiezoController.o PSDInterface.o
	$(CC) -o $(OBJ)$@ -c $(SRC)PiezoControl.cpp $(CFLAGS)

PiezoController.o: binfolder serialib.o 
	$(CC) -o $(OBJ)$@ -c $(SRC)PiezoController.cpp $(CFLAGS)

PSDInterface.o: binfolder
	$(CC) -o $(OBJ)$@ -c $(SRC)PSDInterface.cpp $(CFLAGS)

serialib.o: binfolder
	$(CC) -o $(OBJ)$@ -c $(SRC)serialib.cpp $(CFLAGS)

binfolder:
	if test -d bin;\
		then echo "bin already exists, skipping";\
	else mkdir bin;\
	fi

.PHONY: clean

clean:	
	rm exec
	rm -rf bin
