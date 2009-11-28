// testporject.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <memory>

using namespace std;

int count = 0;

class Test {
public:
	int id;
	int a;
	Test(const Test &b) {id = count++; a=b.a; cout <<"Copying " << id << "\n";}
	Test(int b) {id = count++; a=b;cout <<"Constructing " << id << "\n";}
	~Test() {cout <<"Deconstructing " << id << "\n";}
};

auto_ptr<Test> makeTest() {	
	Test *test = new Test(1);
	
	auto_ptr<Test> t(test);	
	return t;
}

int _tmain(int argc, _TCHAR* argv[])
{
	cout << "Enter Scope\n";
	{
		cout << "Enter Function Scope\n";
		//auto_ptr<Test> t_p = makeTest();
		Test t = *makeTest();
		cout << "Exit Function Scope\n";
		cout << "Got result, t = " << t.a << "\n";

	}
	cout << "Exit Scope\n";
	char abc;
	cin >> abc;
	return 0;
}

