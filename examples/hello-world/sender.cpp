/*
 * This is example source code for use of WFC.
 *
 * Feel free to use and modify as you like.
 */


#include "letter_s.h"

#include <iostream>
#include <fstream>

using namespace std;

int main()
{
	// data that needs to be handled
	string from,to,text;
	unsigned postage;

	// we fill the data
	cout << "from? " << endl;
	cin >> from;
	cout << "to? " << endl;
	cin >> to;
	cout << "text? " << endl;
	cin >> text;
	cout << "postage? " << endl;
	cin >> postage;

	// now the message 'letter' is filled with data
	letter l;
	l.set_sender(from);
	l.set_receiver(to);
	l.set_text(text);
	l.set_postage(postage);

	// serialize to string
	string envelope;
	l.toString(envelope);

	// write strint to file
	fstream f;
	f.open("myletter.bin",ios_base::out|ios_base::binary|ios_base::trunc);
	f << envelope;
	f.close();
}
