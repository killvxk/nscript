// Testbench.cpp : Defines the entry point for the console application.
//

#include "pch.h"
#include "../NScriptHost/NScript3/NScript3.h"

using namespace std;

int main()
{
	nscript3::nscript ns;
	auto v = ns.eval("3+x.y-2");
	cout << to_string(v) << endl;
	auto ei = ns.get_error_info();
	return 0;
}

