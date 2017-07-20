
    ###########################################
    #Makefile for simple programs
    ###########################################
INC=
LIB=
CC=g++ -std=c++0x
# display all warnings
CC_FLAG=-Wall

PRG=mem_pool_test
OBJ=test.o

$(PRG):$(OBJ)
	$(CC) $(INC) $(LIB) -o $@ $(OBJ)

.SUFFIXES: .c .o .cpp
.cpp.o:
	$(CC) $(CC_FLAG) $(INC) -c $*.cpp -o $*.o

.PRONY:clean
clean:
	@echo "Removing linked and compiled files......"
	rm -f $(OBJ) $(PRG)
