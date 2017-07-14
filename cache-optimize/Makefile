

    ###########################################  
    #Makefile for simple programs  
    ###########################################  
INC= 
LIB= 
# CLS is for L1d optimization
CC=g++ -std=c++0x -DCLS=$$(getconf LEVEL1_DCACHE_LINESIZE)
# display all warnings
CC_FLAG=-Wall
      
PRG=matrix
OBJ=matrix_multiplication.o
      
$(PRG):$(OBJ)  
	$(CC) $(INC) $(LIB) -o $@ $(OBJ)  
          
.SUFFIXES: .c .o .cpp  
.cpp.o:  
	$(CC) $(CC_FLAG) $(INC) -c $*.cpp -o $*.o  
      
.PRONY:clean  
clean:  
	@echo "Removing linked and compiled files......"  
	rm -f $(OBJ) $(PRG)  
