#pragma once

#include <string>

bool download_file(const std::string& url, const std::string& output);
bool download_string(const std::string& url, std::string& output);
