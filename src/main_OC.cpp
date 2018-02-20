#include <cstdio>
#include "DD_Strings.h"
#include "DD_MeshUtility.h"
#include "DD_Container.h"
#include "DD_ObjConverter.h"

// g++ main.cpp -I ./ -ggdb -std=c++11 -o test

int main(int argc, char const *argv[])
{
	if (argc == 2) {
		DD_ObjConverter converter;
		converter.importOBJ(argv[1]);
		converter.printStats();
		converter.exportMesh();
	}
	
	return 0;
}