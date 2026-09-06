#include "csv.hpp"

#include <sstream>

std::vector<std::string> split(
	const std::string& line,
	char delimiter
)
{
	std::vector<std::string> result;
	std::stringstream ss(line);

	std::string item;

	while (std::getline(ss, item, delimiter))
		result.push_back(item);

	return result;
}
