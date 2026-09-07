#include <cstdio>
#include <fstream>
#include <string>
#include <sstream>

#include "repo.hpp"
#include "csv.hpp"
#include "download.hpp"
#include "config.hpp"

std::vector<RepositoryConfig> load_repositories(const std::string& path) {
	std::vector<RepositoryConfig> repos;

	std::ifstream file(path);

	if (!file.is_open()) {
		printf("failed to open %s\n", path.c_str());
		return repos;
	}

	std::string line;

	if (!std::getline(file, line))
		return repos;

	while (std::getline(file, line)) {
		auto fields = split(line, ',');

		if (fields.size() < 2) {
			printf("bad repository entry\n");
			continue;
		}

		RepositoryConfig repo;

		repo.name = fields[0];
		repo.url = fields[1];

		repos.push_back(repo);
	}

	return repos;
}

bool sync_repo(const RepositoryConfig& repo) {
	std::string directory = REPOS_DIRECTORY + repo.name;

	std::string path = directory + "/index.csv";

	std::string command = "mkdir -p \"" + directory + "\"";

	if (std::system(command.c_str()) != 0)
		return false;

	std::string url = repo.url;

	if (!url.empty() && url.back() != '/')
		url += '/';

	url += "index.csv";

	return download_file(url, path);
}

Repository load_repo(const RepositoryConfig& config) {
	Repository repo;

	repo.name = config.name;
	repo.url = config.url;

	std::string path = REPOS_DIRECTORY + config.name + "/index.csv";

	std::ifstream file(path);

	if (!file.is_open()) {
		printf("failed to open %s\n", path.c_str());

		return repo;
	}

	std::string line;

	if (!std::getline(file, line))
		return repo;

	while (std::getline(file, line)) {
		auto fields = split(line, ',');

		if (fields.size() < 2) {
			printf("bad package entry\n");
			continue;
		}

		Package pkg;

		pkg.name = fields[0];
		pkg.meta = fields[1];

		repo.packages[pkg.name] = pkg;
	}

	return repo;
}

Package* find_package(Repository& repo, const std::string& name) {
	auto it = repo.packages.find(name);

	if (it == repo.packages.end())
		return nullptr;

	return &it->second;
}

PackageMeta load_package_meta( const Repository& repo, const Package& pkg) {
	PackageMeta meta;

	std::string url = repo.url;

	if (!url.empty() && url.back() != '/')
		url += '/';

	url += pkg.meta;

	std::string data;

	if (!download_string(url, data)) {
		printf("failed to download metadata for %s\n", pkg.name.c_str());

		return meta;
	}

	std::stringstream stream(data);
	std::string line;

	while (std::getline(stream, line)) {
		auto fields = split(line, ',');

		if (fields.size() < 2)
			continue;

		if (fields[0] == "description")
			meta.description = fields[1];
		else if (fields[0] == "version")
			meta.version = fields[1];
		else if (fields[0] == "depends" && !fields[1].empty())
		    meta.depends = split(fields[1], ';');
	}

	return meta;
}
