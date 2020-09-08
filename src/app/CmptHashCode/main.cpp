#include <UECS/CmptType.h>

#include <iostream>

using namespace std;

int main(int argc, char** argv) {
	if (argc != 2)
		return 1;

	std::cout << Ubpa::UECS::CmptType(argv[1]).HashCode() << std::endl;

	return 0;
}
