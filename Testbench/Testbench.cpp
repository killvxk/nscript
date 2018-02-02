// Testbench.cpp : Defines the entry point for the console application.
//

#include "pch.h"
#include "../NScriptHost/NScript3/NScript3.h"

using namespace std;

int main()
{
	nscript3::NScript ns;
	error_code ec;
	auto v = ns.eval("intr = sub(f,a,b,dx) {for(my s=0, my x=a;x<b;x+=dx) s+=f(x)*dx; s}; intr(sub(x) x^2, 0, 2, 0.01)", ec);
	//auto v = ns.eval("sub {x=1;}", ec);
	//auto v = ns.eval("-3", ec);
	cout << to_string(v) << endl;
	return 0;
}

