#include <fstream>
#include <iostream>
#include <iterator>

#include "interpreter.hpp"

std::string readFile(const std::string &filename)
{
	std::ifstream file(filename);
	if (!file) throw "Error opening input file";
	file.unsetf(std::ios_base::skipws);
	std::istream_iterator<char> begin(file), end;
	std::string buffer(begin, end);
	return buffer;
}


void printUsage(const char *arg0)
{
	printf("%s - a z80 interpreter\n", arg0);
	printf("\n");
	printf("usage: %s file\n", arg0);
	printf("\n");
	printf("file - the z80 source to run\n");
	printf("\n");
}

int main(int argc, char **argv)
{
	if (argc != 2) {
		printUsage(argv[0]);
		return 1;
	}

	try {
		std::string source = readFile(argv[1]);
		tokeniseSource(source);
	} catch(const char *e) {
		std::cerr << e << "\n";
		return 1;
	}
}
