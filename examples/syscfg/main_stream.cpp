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
		cout << "\nwriting config successful\n";
	else
		cerr << "\nerror writing config\n";
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


static uint32_t str_to_ip(const char *str)
{
	uint8_t b[4];
	int n = sscanf(str,"%hhu.%hhu.%hhu.%hhu",b+0,b+1,b+2,b+3);
	if (4 == n) {
		uint32_t ip = ((uint32_t)b[0]<<24) | ((uint32_t)b[1]<<16) | ((uint32_t)b[2]<<8) | (uint32_t)b[3];
		return ip;
	}
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

	cout << "\nNow continue interactively, by setting gateway and netmask.\n"
		"Therefore, enter parameter name and its argument separated by a singel space.\n"
		"e.g. type 'netmask 24'\n";
	char line[80];
	do {
		cin.getline(line,sizeof(line));
		char *sp = strchr(line,' ');
		if (sp) {
			*sp = 0;
			if (0 > cfg.setByName(line,sp+1))
				cerr << "error parsing '" << line << "' = '" << sp+1 << "'\n";
		}
	} while (line[0] != 0);
	
	// without setByName
	/*
	char line[80];
	do {

		// this code misses necessary checks to be more readable as an example
		if (!memcmp(line,"gateway ",8)) {
			uint32_t ip = str_to_ip(line+8);
			if (ip)
				cfg.set_gateway(ip);
			else
				cerr << "invalid IP address\n";
		} else if (!memcmp(line,"ipv4 ",5)) {
			uint32_t ip = str_to_ip(line+5);
			if (ip)
				cfg.set_ipv4(ip);
			else
				cerr << "invalid IP address '" << line+4 << "'\n";
		} else if (!memcmp(line,"netmask ",8)) {
			long v = strtol(line+8,0,0);
			cfg.set_netmask(v);
		} else if (line[0] != 0) {
			cerr << "invalid parameter\n";
		}
		// ...
	} while (line[0] != 0);
	*/
	
	cfg.toASCII(std::cout);
	cout << "\n";
	// save the config before terminating
	saveConfig(cfg);
	return 0;
}
