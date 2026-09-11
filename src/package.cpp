#include "package.hpp"

#include <cstdio>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

#include "models.hpp"
#include "repo.hpp"
#include "csv.hpp"
#include "download.hpp"
#include "config.hpp"

bool download_package(const Repository& repo, const Package& pkg, const std::string& output) {
	std::string url = repo.url;

	if (!url.empty() && url.back() != '/')
		url += '/';

	url += pkg.metadata.content;

	printf("downloading %s\n", url.c_str());

	if (!download_file(url, output)) {
		std::remove(output.c_str());
		return false;
	}

	return true;
}

Package* find_package(Repository& repo, const std::string& name) {
	auto it = repo.packages.find(name);

	if (it == repo.packages.end())
		return nullptr;

	return &it->second;
}

bool is_installed(const std::string& name) {
	std::ifstream file(INSTALLED_PATH);
	std::string line;

	if (!std::getline(file, line))
		return false;

	while (std::getline(file, line)) {
		auto fields = split(line, ',');

		if (fields.size() >= 1 && fields[0] == name)
			return true;
	}

	return false;
}

std::vector<Package> get_installed() {
	std::vector<Package> packages;
	std::ifstream file(INSTALLED_PATH);
	std::string line;

	if (!file.is_open())
		return packages;

	std::getline(file, line);

	while (std::getline(file, line)) {
		auto fields = split(line, ',');

		if (fields.size() < 3)
			continue;

		Package pkg;
		pkg.name = fields[0];
		pkg.metadata.version = fields[1];

		packages.push_back(pkg);
	}

	return packages;
}

bool remove_package(const std::string& name) {
	std::ifstream input(INSTALLED_PATH);

	if (!input.is_open())
		return false;

	std::vector<std::string> entries;
	std::string line;

	if (std::getline(input, line))
		entries.push_back(line);

	while (std::getline(input, line)) {
		auto fields = split(line, ',');

		if (fields.size() < 3) {
			entries.push_back(line);
			continue;
		}

		if (fields[0] != name) {
			entries.push_back(line);
			continue;
		}

		if (std::remove(fields[2].c_str()) != 0)
			return false;
	}

	input.close();

	std::ofstream output(INSTALLED_PATH);

	if (!output.is_open())
		return false;

	for (auto& entry : entries)
		output << entry << '\n';

	return true;
}
