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
	string operator() (nscript3::date_t dt) {

		auto t = std::chrono::system_clock::to_time_t(dt);
		tm tm;
		localtime_s(&tm, &t);
		stringstream ss;
		if(tm.tm_hour || tm.tm_min || tm.tm_sec)
			ss << std::put_time(&tm, tm.tm_sec ? "%d.%m.%Y %H:%M:%S" : "%d.%m.%Y %H:%M");
		else
			ss << std::put_time(&tm, "%d.%m.%Y");
		return ss.str();
	}
	string operator() (nscript3::array_t a) { return "array"; }
	string operator() (nscript3::object_ptr o) { auto v = o->get(); return "object: " + std::visit(print_value(), v); }
};


int main()
{
	nscript3::NScript ns;
	error_code ec;
	auto v = ns.eval("#26.10.74#", ec);
	//auto v = ns.eval("-3", ec);
	cout << std::visit(print_value(), v) << endl;
	return 0;
}

