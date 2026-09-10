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


bool sync_repo(const RepositoryConfig& repo);

bool download_package(const Repository& repo, const Package& pkg, const std::string& output);
Package* find_package(Repository& repo, const std::string& name);

std::vector<RepositoryConfig> load_repos(const std::string& path);
Repository load_repo(const RepositoryConfig& repo);
PackageMeta load_package_meta(const Repository& repo, const Package& pkg);
