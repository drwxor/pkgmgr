#pragma once

#include <string>
#include <vector>
#include <unordered_map>

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

struct Repository {
	std::string name;
	std::string url;
	std::unordered_map<std::string, Package> packages;
};

struct RepositoryConfig {
	std::string name;
	std::string url;
};
