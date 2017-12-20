###########################################
#Makefile for simple programs
###########################################
INC= -I../../ThirdParty
LIB= -lpthread

CC=g++ -std=c++0x
# display all warnings
CC_FLAG=-Wall

# 生成的可执行文件名
PRG=PathPlanner
# 用到的 .o
OBJ=main.o path_planner.o ../../ThirdParty/inih/ini.o ../../ThirdParty/inih/cpp/INIReader.o

$(PRG):$(OBJ)
	$(CC) $(INC) $(LIB) -o $@ $(OBJ)

.SUFFIXES: .c .o .cpp
.cpp.o:
	$(CC) $(CC_FLAG) $(INC) -c $*.cpp -o $*.o

.PRONY:clean
clean:
	@echo "Removing linked and compiled files......"
	rm -f $(OBJ) $(PRG)
