#include "util.hpp"

namespace Util {

const std::string readFromFile(const std::string& path) {
	std::string content;
	std::ifstream fileStream(path, std::ios::in);

	if (!fileStream.is_open()) {
		std::cout << OUT_ERROR << "Could not open file: " << path << std::endl;
		return "";
	}

	std::string line = "";
	while (!fileStream.eof()) {
		std::getline(fileStream, line);
		content.append(line + "\n");
	}

	fileStream.close();
	return content;
}

}