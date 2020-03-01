#include "novarint.h"
#include "runcheck.h"
#include "runcheck.cpp"

int main()
{
	NoVarint nv;
	runcheck(nv);
	nv.set_ip4(192 | (168<<8) | (1<<16) | (32<<24));
	nv.set_netmask(24);
	runcheck(nv);
	nv.set_gateway(192 | (168<<8) | (1<<16) | (1<<24));
	nv.set_dns(192 | (168<<8) | (1<<16) | (2<<24));
	runcheck(nv);
}
