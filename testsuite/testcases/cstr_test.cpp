#ifdef __AVR__
#include "nodeinfo_avr.h"
#else
#include <string>
#include <stdio.h>
#include <iostream>
#include "NodeInfo.h"
#endif

#include "runcheck.cpp"


int main(int argc, const char *argv[])
{
	char hash[] = {9,8,7,6,5,4,3,2,1,0,1,2,3,4,5,7,8,9};
	NodeInfo ni;
#ifdef __AVR__
	uart_config();
	uart_test();
	while (1) {
	uart_send_p(PSTR("start\n\r"));

	uart_send_char('O');
	uart_send_char('K');
	uart_send_char('\n');
	uart_send_char('\r');
	ni.set_node(CString(PSTR("Node #1"),8,true));
	ni.set_hash(hash,sizeof(hash));
	Port *p = ni.add_ports();
	//p->set_id(CString("port PA1"));
	p->set_id(CString(PSTR("port PA1"),9,true));
	p->set_name(CString(PSTR("ambient light A/D"),18,true));
	p = ni.add_ports();
	p->set_id(CString(PSTR("port PB1"),9,true));
	p->set_name(CString(PSTR("digital switch #1"),18,true));
	runcheck(ni);
	uart_send_p(PSTR("OK\n\r"));
	_delay_us(1000000);
	}
#else
	ni.set_node(CString(PSTR("Node #1"),8));
	ni.set_hash(hash,sizeof(hash));
	Port *p = ni.add_ports();
	//p->set_id(CString("port PA1"));
	p->set_id(PSTR("port PA1"));
	//p->set_name(PSTR("ambient light A/D"));
	p->set_name(PSTR("A/D"));
	p = ni.add_ports();
	p->set_id(PSTR("port PB1"));
	//p->set_name(PSTR("digital switch #1"));
	p->set_name(PSTR("#1"));
	runcheck(ni);
	printf("%s: %s\n",argv[0],testcnt());
	return 0;
#endif
}
