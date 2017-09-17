#Makefile
#Created on: Sep 13, 2017
#   Author: nafiu

CC=g++
LIB=-lmath
OPTIONS=-g -Wall -pg

all:cpu

cpu:cpu.cpp cpu.o
	$(CC) $(OPTIONS) -o ./cpu ./mycpu.cpp ./cpu.o
	
cpu.o:cpu.cpp
	$(CC) $(OPTIONS) -c ./cpu.cpp
	
clean:
	rm -rf ./cpu.o ./cpu