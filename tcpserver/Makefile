###########################################
#Makefile for simple programs
###########################################
INC=
LIBPATH=
LIB=-lpthread
CC=g++ -std=c++14
# display all warnings
CC_FLAG=-Wall -g -O2

PRG=TcpServer
OBJ=main.o tcp_server.o

$(PRG):$(OBJ)
	$(CC) $(INC) -o $@ $(OBJ) $(LIBPATH) $(LIB)

.SUFFIXES: .c .o .cpp
.cpp.o:
	$(CC) $(CC_FLAG) $(INC) -c $*.cpp -o $*.o

.PRONY:clean
clean:
	@echo "Removing linked and compiled files......"
	rm -f $(OBJ) $(PRG)
