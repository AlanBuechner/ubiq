#pragma once
#include <string>
#include <vector>
#include <queue>

// helper functions
bool isToken(char c, const std::vector<char> tokens);

std::queue<std::string> Tokenize(const std::string& str,
	const std::vector<char> seperatingTokens);

template <typename T> std::vector<T> QueueToVector(std::queue<T> q) {
	std::vector<T> elems;
	elems.reserve(q.size());
	while (!q.empty()) {
		elems.push_back(q.front());
		q.pop();
	}
	return elems;
}

template <typename T>
bool hasEntry(const std::vector<T>& list, const T& entry) {
	for (auto& e : list) {
		if (e == entry)
			return true;
	}
	return false;
}
