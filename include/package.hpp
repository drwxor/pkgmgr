#pragma once

#include <string>
#include <vector>

#include "models.hpp"

bool download_package(const Repository& repo, const Package& pkg, const std::string& output);
Package* find_package(Repository& repo, const std::string& name);
bool is_installed(const std::string& name);
std::vector<Package> get_installed();
bool remove_package(const std::string& name);
