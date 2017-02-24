#include <fstream>
#include <iostream>
#include <iterator>
#include <unistd.h>

#include "register_machine.hpp"
#include "stack_machine.hpp"
#include "register_convert.hpp"

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
	static const char *USAGE = ""
		"%s - an interpreter of some sort.\n"
		"Does something with stacks\n"
		"\n"
		"Usage: %s [-v] [-s] -i file\n"
		"\n"
		"-v    -  Verbose output\n"
		"-c    -  Convert register code\n"
		"-s    -  Stack (J5) interpreter\n"
		"-r    -  Register (DCPU-16) interpreter\n"
		"-h    -  This help text\n"
		"file  -  ASM source file to run\n";
	printf(USAGE, arg0, arg0);
}

enum class mode {
	REGISTER,
	STACK,
	CONVERT,
};

int main(int argc, char **argv)
{
	bool verbose = false;
	mode m;
	const char *filepath = "";
	int c = 0;
	while ((c = getopt(argc, argv, "hvcsri:")) != -1) {
		switch (c) {
			case 'v':
				verbose = true;
				break;
			case 'c':
				m = mode::CONVERT;
				break;
			case 's':
				m = mode::STACK;
				break;
			case 'r':
				m = mode::REGISTER;
				break;
			case 'i':
				filepath = optarg;
				break;
			case 'h':
				printUsage(argv[0]);
				return 0;
			default:
				printUsage(argv[0]);
				return 1;
		}
	}
	if (strcmp(filepath, "") == 0) {
		printUsage(argv[0]);
		return 1;
	}

	try {
		std::string source = readFile(filepath);

		switch (m) {
			case mode::REGISTER: {
				dcpu16::program prog = dcpu16::tokenise_source(source);
				if (verbose) {
					for (const auto &ins : prog) std::cout << ins << "\n";
				}
				dcpu16::machine mach;
				mach.run(prog, verbose);

				break;
			}
			case mode::STACK: {
				j5::program prog = j5::tokenise_source(source);
				if (verbose) {
					for (const auto &ins : prog) std::cout << ins << "\n";
				}
				j5::machine mach;
				mach.run(prog, verbose);
				break;
			}
			case mode::CONVERT: {
				dcpu16::program prog = dcpu16::tokenise_source(source);
				if (verbose) {
					for (const auto &ins : prog) std::cout << ins << "\n";
				}
				auto stack_prog = reg2stack(prog);
				for (const auto &i : stack_prog) std::cout << i << '\n';
				break;
			}
		}
		std::cout << '\n';

	} catch(const char *e) {
		std::cerr << e << "\n";
		return 1;
	} catch(const std::string &e) {
		std::cerr << e << "\n";
		return 1;
	}
}
