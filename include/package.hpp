#pragma once

#include <string>
#include <vector>

#include "models.hpp"

bool download_package(const Repository& repo, const Package& pkg, const std::string& output);
Package* find_package(Repository& repo, const std::string& name);
bool is_installed(const std::string& name);
std::vector<Package> get_installed();
bool remove_package(const std::string& name);
bool install_package_from_repo(Repository* repo, const Package* pkg);
bool install_package(const char *name, std::vector<RepositoryConfig>& repos);
