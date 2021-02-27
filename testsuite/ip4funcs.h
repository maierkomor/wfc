#include <iostream>
#include <stdlib.h>

inline void ip4_ascii(std::ostream &str, uint32_t ip)
{
	str << (ip & 0xff) << '.' << ((ip >> 8) & 0xff) << '.' << ((ip >> 16) & 0xff) << '.' << ((ip >> 24 ) & 0xff);
}


static int parse_ipv4(uint32_t *ip, const char *str)
{
	uint8_t b[4];
	int n = sscanf(str,"%hhu.%hhu.%hhu.%hhu",b+0,b+1,b+2,b+3);
	if (4 == n) {
		*ip = ((uint32_t)b[0]) | ((uint32_t)b[1]<<8) | ((uint32_t)b[2]<<16) | ((uint32_t)b[3]<<24);
		return 1;
	}
	std::cerr << "parser error " << n << "\n";
	return -1;
}
