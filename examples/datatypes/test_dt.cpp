#include "datatypes.h"
#include <iostream>

using namespace std;

int main()
{
	DataTypes D;

	D.mutable_fs() = "some string";
	cout << D.fs().c_str() << endl;
	
	D.mutable_fs() = "a way too long string that will be truncated during assignment";
	cout << D.fs().c_str() << endl;
}
