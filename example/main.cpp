#include "divisi.h"
#include <iostream>

int main(int argc, const char* argv[])
{
	if(argc < 2){
		std::cerr << "Need RPGMaker-VXAce Project Path" << std::endl;
		return -1;
	}

	langscore::divisi divisi(argv[0]);

	divisi.setProjectPath(argv[1]);

	divisi.exec();

	return 0;
}