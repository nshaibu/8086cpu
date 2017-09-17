//============================================================================
// Name        : mycpu.cpp
// Author      : nafiu
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include "./cpu.h"
using namespace std;

int main(int argc, char **argv) {
	_8086cpu mycpu;
	unsigned int size;

	mycpu.load("/home/nafiu/Desktop/hello", size);
	cout << size;
	mycpu.running(size);
	mycpu.debug_cpu();
}
