#pragma once

#include <string>
#include <vector>

struct Package {
	std::string name;
	std::string meta;
};

struct PackageMeta {
	std::string description;
	std::string version;
	std::string content;
	std::vector<std::string> depends;
};
