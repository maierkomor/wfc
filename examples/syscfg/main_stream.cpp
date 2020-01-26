// example using file i/o streams

#include "syscfg_pc.h"

#include <fstream>	// for file I/O
#include <iostream>	// for JSON output

using namespace std;

static char CfgName[] = "syscfg.bin";


void initDefaults(Config &cfg)
{
	cout << "initializing defaults\n";
	cfg.set_hostname("host001");
	cfg.set_baudrate(115200);
	cfg.set_wifi_ssid("mywifi");
	cfg.set_wifi_pass("1234567890");
}


void saveConfig(const Config &cfg)
{
	string str;
	cfg.toString(str);	// serialize the data

	ofstream out;
	out.open(CfgName);	// open file
	out << str;		// write to file
	if (out.good())
		cout << "writing config successful\n";
	else
		cerr << "error writing config\n";
}


int readConfig(Config &cfg)
{
	ifstream in;
	in.open(CfgName,ios::in);	// open file
	in.seekg(0,ios::end);		// seek to end...
	ssize_t n = in.tellg();		// ... to determine size
	in.seekg(0,ios::beg);		// back to begin
	if (n <= 0)
		return 1;
	char *buf = new char[n];	// allocate buffer
	in.read(buf,n);			// read data
	
	cfg.fromMemory(buf,n);		// here we parse the buffer
	delete[] buf;			// clean-up
	// That's it! Ready to go.
	
	cout << "configuration restored from " << CfgName << endl;
	return 0;
}


int main()
{
	Config cfg;
	if (readConfig(cfg)) {
		// Config could not be read.
		// So we initialize it with defaults.
		cout << "unable to read " << CfgName << ", creating with defaults\n";
		initDefaults(cfg);
	}

	// do some work here
	// e.g. produce ASCII output
	cfg.toASCII(std::cout);
	
	// save the config before terminating
	saveConfig(cfg);
	return 0;
}
