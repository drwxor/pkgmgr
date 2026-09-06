#pragma once

#include <vector>

#include "package.hpp"

std::vector<Package> load_repo(const std::string& path);

Package* find_package(
	std::vector<Package>& packages,
	const std::string& name
);
