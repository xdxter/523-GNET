// test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>

using namespace std;

struct IStruct {
	int a;
};

struct StructB : IStruct {
	int b;
};

int _tmain(int argc, _TCHAR* argv[])
{
	StructB thing;
	thing.a = 1; thing.b = 2;
	IStruct *one = new IStruct(thing);
	cout << one->a << "," << static_cast<StructB*>(one)->b << endl;

	char a;
	cin >> a;
	return 0;
}

