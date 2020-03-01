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
	nc.set_hostname("somehost");
	nc.set_address(ip_to_int(192,168,1,60));
	nc.set_netmask(24);
	nc.set_gateway(ip_to_int(192,168,1,1));
	nc.set_dns(ip_to_int(192,168,1,2));
	nc.set_degC(21.2);

	// create JSON output
	nc.toJSON(cout);
	cout << endl;

	// print ASCII representation
	nc.toASCII(cout);

	// modification of contents can also be done by setting by name
	nc.setByName("hostname","myhost");
	nc.setByName("degC","19.8");
	nc.toASCII(cout);

	// binary output:
	cout << "encoded binary has " << nc.calcSize() << " bytes\n"
		"binary output:\n\n";

	uint8_t buf[nc.calcSize()];
	nc.toMemory(buf,sizeof(buf));	// serialze to buffer

	// print hex representation of buffer
	for (int i = 0; i < sizeof(buf); )
		printf("%02x%c", buf[i], ((++i)&0xf) ? ' ': '\n');
	printf("\n");
}
