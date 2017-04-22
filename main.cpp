#include <fstream>
#include <iostream>
#include <iterator>
#include <unistd.h>

#include "convert_machine.hpp"
#include "register_machine.hpp"
#include "stack_machine.hpp"
#include "util.hpp"

log_level_t GLOBAL_LOG_LEVEL = LOG_INFO; // default, not actually a constant

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
		"Usage: %s [-v lvl] [-f] [-o num] [-scr] file\n"
		"\n"
		"-v lvl  -  Output verbosity - 0-3\n"
		"-f      -  Fast speed\n"
		"-o num  -  Only has affect with -c. Level 1 indicates single\n"
		"           peephole pass, Level 2 does Koopman-style optimisation\n"
		"-c      -  Convert register code\n"
		"-s      -  Stack (J5) interpreter\n"
		"-r      -  Register (DCPU-16) interpreter\n"
		"-h      -  This help text\n"
		"file    -  ASM source file to run\n";
	printf(USAGE, arg0, arg0);
}

enum class mode {
	REGISTER,
	STACK,
	CONVERT,
};

int main(int argc, char **argv)
{
	bool speedlimit = true;
	size_t optimise = 0;
	bool nocache = false;
	mode m;
	const char *filepath = "";
	int c = 0;
	while ((c = getopt(argc, argv, "hnfv:o:c:s:r:")) != -1) {
		switch (c) {
			case 'v':
				GLOBAL_LOG_LEVEL = static_cast<log_level_t>(atoi(optarg));
				break;
			case 'c':
				m = mode::CONVERT;
				filepath = optarg;
				break;
			case 's':
				m = mode::STACK;
				filepath = optarg;
				break;
			case 'r':
				m = mode::REGISTER;
				filepath = optarg;
				break;
			case 'f':
				speedlimit = false;
				break;
			case 'n':
				nocache = true;
				break;
			case 'o':
				optimise = atoi(optarg);
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
				for (const auto &ins : prog) log<LOG_INFO>(ins);
				dcpu16::machine mach;
				mach.run(prog, speedlimit);

				break;
			}
			case mode::STACK: {
				j5::program prog = j5::tokenise_source(source);
				for (const auto &ins : prog) log<LOG_INFO>(ins);
				j5::machine mach;
				mach.run(prog, speedlimit);
				break;
			}
			case mode::CONVERT: {
				dcpu16::program prog = dcpu16::tokenise_source(source);
				for (const auto &ins : prog) log<LOG_INFO>(ins);
				convertmachine mach;
				mach.run_reg(prog, speedlimit, optimise, !nocache);
				break;
			}
		}
		std::cout << '\n';

	} catch(const char *e) {
		log<LOG_NOTHING>(e);
		return 1;
	} catch(const std::string &e) {
		log<LOG_NOTHING>(e);
		return 1;
	}
}
