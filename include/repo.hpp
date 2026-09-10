#pragma once

#include <string>
#include <vector>

#include "models.hpp"

bool sync_repo(const RepositoryConfig& repo);
std::vector<RepositoryConfig> load_repos(const std::string& path);
Repository load_repo(const RepositoryConfig& repo);
PackageMeta load_package_meta(const Repository& repo, const Package& pkg);
