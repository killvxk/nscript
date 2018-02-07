// Testbench.cpp : Defines the entry point for the console application.
//

#include "pch.h"
#include "../NScriptHost/NScript3/NScript3.h"

using namespace std;

int main()
{
	nscript3::nscript ns;
	auto v = ns.eval("hour(#13:34#)");
	v = ns.eval("#16.10.1974#");
	//auto v = ns.eval("sub {x=1;}", ec);
	//auto v = ns.eval("-3", ec);
	cout << to_string(v) << endl;
	return 0;
}

