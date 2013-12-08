#include "Matrix.h"
#include <iostream>

using std::cout;

int main(int argc, const char *argv[])
{
	static double m[9] = {
		1.0, 2.0, 3.0,
		4.0, 5.0, 6.0,
		7.0, 8.0, 9.0
	};
	
	Matrix3d m3d = Matrix3d(m);
	
	cout << m3d.serialize() << std::endl;
	
	m3d[0][1] = 0;

	cout << m3d.serialize() << std::endl;

	cout << m3d.transposed().serialize() << std::endl;

	return 0;
}
