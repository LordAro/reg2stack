#include "util.hpp"

std::vector<std::string> split_words(const std::string &line)
{
	// Parse into separate words
	std::vector<std::string> words;
	bool inword = false;
	bool inquote = false;
	for (char c : line) {
		if (c == ';') break; // comment
		if (c == '"') inquote = !inquote;
		if (!inquote && (isspace(c) || c == ',')) {
			inword = false;
			continue;
		} else if (!inword) {
			words.emplace_back(); // start next word
		}
		words.back().push_back(c);
		inword = true;
	}
	return words;
}

