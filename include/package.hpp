#pragma once

#include <string>
#include <vector>

struct PackageMeta {
	std::string description;
	std::string version;
	std::string content;
	std::vector<std::string> depends;
};

struct Package {
	std::string name;
	std::string meta;

	PackageMeta metadata;
};
