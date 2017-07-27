
    ###########################################
    #Makefile for simple programs
    ###########################################
INC=
LIB=-lpthread
CC=g++ -std=c++0x
# display all warnings
CC_FLAG=-Wall

PRG=ringbuf
OBJ=main.o ringbuffer.o

$(PRG):$(OBJ)
	$(CC) $(INC) -o $@ $(OBJ) $(LIB)

.SUFFIXES: .c .o .cpp
.cpp.o:
	$(CC) $(CC_FLAG) $(INC) -c $*.cpp -o $*.o

.PRONY:clean
clean:
	@echo "Removing linked and compiled files......"
	rm -f $(OBJ) $(PRG)
