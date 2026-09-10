#pragma once

#include <string>
#include <vector>

bool extract_package(const std::string& archive,const std::string& destination, std::vector<std::string>& files);
