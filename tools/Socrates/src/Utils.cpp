#include "Utils.h"

// helper functions
bool isToken(char c, const std::vector<char> tokens) {
	for (uint32_t i = 0; i < tokens.size(); i++) {
		if (c == tokens[i])
			return true;
	}
	return false;
}

std::queue<std::string> Tokenize(const std::string& str,
	const std::vector<char> seperatingTokens) {
	std::queue<std::string> tokens;

	uint32_t lastTokenLocation = 0;
	for (uint32_t i = 0; i < str.size(); i++) {
		if (isToken(str[i], seperatingTokens)) {
			tokens.push(str.substr(lastTokenLocation, i - lastTokenLocation));
			tokens.push(std::string(1, str[i]));
			lastTokenLocation = i + 1;
		}
	}
	tokens.push(str.substr(lastTokenLocation, str.size() - lastTokenLocation));
	return tokens;
}
