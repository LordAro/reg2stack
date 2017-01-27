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

	z80machine test;
	boost::variant<uint8_t, uint16_t> r;
	std::cout << (int)test.main['a'] << "\n";
	test.main['a'] = 5;
	r = test.main['a'];
	r = (uint8_t)7;
	std::cout << (int)test.main['a'] << "\n";

	try {
		std::string source = readFile(argv[1]);
		z80prog prog = tokeniseSource(source);

		for (const auto &i : prog) {
			if (!i.label.empty()) std::cout << i.label << ": ";
			std::cout << op_t_str.at((size_t)i.op) << ' ';
			if (!i.operand1.empty()) std::cout << i.operand1;
			if (!i.operand2.empty()) std::cout << ',' << i.operand2;
			std::cout << "\n";
		}
	} catch(const char *e) {
		std::cerr << e << "\n";
		return 1;
	}
}
