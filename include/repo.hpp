#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "package.hpp"

struct Repository {
	std::string name;
	std::string url;
	std::unordered_map<std::string, Package> packages;
};

struct RepositoryConfig {
	std::string name;
	std::string url;
};

std::vector<RepositoryConfig> load_repositories(const std::string& path);

bool sync_repo(const RepositoryConfig& repo);

Repository load_repo(const RepositoryConfig& repo);

Package* find_package(Repository& repo, const std::string& name);

PackageMeta load_package_meta(const Repository& repo, const Package& pkg);
