// Testbench.cpp : Defines the entry point for the console application.
//

#include "pch.h"
#include "../NScriptHost/NScript3/NScript3.h"

using namespace std;

struct print_value {
	string operator() (int i) { return to_string(i); }
	string operator() (double d) {
		stringstream ss;
		ss << d;
		return ss.str();
	}
	string operator() (string s) { return s; }
	string operator() (nscript3::date_t dt) { return "date"; }
	string operator() (nscript3::array_t a) { return "array"; }
	string operator() (nscript3::object_ptr o) { return "object"; }
};


int main()
{
	nscript3::NScript ns;
	auto v = ns.eval("(3+2*2-4.7) * 3");
	cout << std::visit(print_value(), v) << endl;
	return 0;
}

