#pragma once

#include <string>

#include "models.hpp"

bool download_package(const Repository& repo, const Package& pkg, const std::string& output);
bool remove_package(const std::string& name);
Package* find_package(Repository& repo, const std::string& name);
