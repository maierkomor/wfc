#include "netconfig.h"
#include <iostream>
#include <stdint.h>

using namespace std;

void uint32_to_ip4(std::ostream &o, uint32_t v)
{
	o	<< (unsigned) (v&0xff)
		<< '.'
		<< (unsigned) ((v>>8)&0xff)
		<< '.'
		<< (unsigned) ((v>>16)&0xff)
		<< '.'
		<< (unsigned) ((v>>24)&0xff)
		;
}


int parse_ip4(uint32_t *ip4, const char *str)
{
	uint8_t ip[4];
	int n;
	int r = sscanf(str,"%hhu.%hhu.%hhu.%hhu%n",ip,ip+1,ip+2,ip+3,&n);
	if (4 != r) {
		printf("sscanf returned %d\n",r);
		return -1;
	}
	printf("parsed %d chars: %hhu.%hhu.%hhu.%hhu\n",n,ip[0],ip[1],ip[2],ip[3]);
	*ip4 = ip[0] | (ip[1] << 8) | (ip[2] << 16) | (ip[3] << 24);
	return n;
}

void degC_to_json(std::ostream &o, float f)
{
	o.precision(3);
	o << f;
	o << "\\u00b0C";
}

void degC_to_ascii(std::ostream &o, float f)
{
	o.precision(3);
	o << f;
	o << "\u00b0C";
}

uint32_t ip_to_int(uint8_t a0, uint8_t a1, uint8_t a2, uint8_t a3)
{
	return a0 | (a1<<8) | (a2<<16) | (a3<<24);
}

int main()
{
	NetConfig nc;
	string tmp;

	cout << "hostname?\n";
	cin >> tmp;
	nc.set_hostname(tmp);

	cout << "address?\n";
	cin >> tmp;
	int r = nc.setByName("address",tmp.c_str());
	cout << "set returned " << r << endl;

	nc.set_netmask(24);
	nc.set_gateway(ip_to_int(192,168,1,1));
	nc.set_dns(ip_to_int(192,168,1,2));
	nc.set_degC(21.2);

	// create JSON output
	nc.toJSON(cout);
	cout << endl;

	// print ASCII representation
	nc.toASCII(cout);
	cout << endl;

	// modification of contents can also be done by setting by name
	nc.setByName("hostname","myhost");
	nc.setByName("degC","19.8");
	nc.toASCII(cout);
	cout << endl;
	cout << endl;

	// binary output:
	cout << "encoded binary has " << nc.calcSize() << " bytes\n"
		"binary output:\n";

	uint8_t buf[nc.calcSize()];
	nc.toMemory(buf,sizeof(buf));	// serialze to buffer

	// print hex representation of buffer
	for (int i = 0; i < sizeof(buf); )
		printf("%02x%c", buf[i], ((++i)&0xf) ? ' ': '\n');
	printf("\n");
}
