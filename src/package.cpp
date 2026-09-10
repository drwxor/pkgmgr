#include "package.hpp"

#include <cstdio>
#include <fstream>
#include <string>
#include <sstream>

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
