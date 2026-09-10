#include <cstdio>
#include <fstream>
#include <string>
#include <sstream>

#include "repo.hpp"
#include "csv.hpp"
#include "download.hpp"
#include "config.hpp"

bool sync_repo(const RepositoryConfig& repo) {
	std::string directory = REPOS_DIRECTORY + repo.name;

	std::string path = directory + "/index.csv";

	std::string command = "mkdir -p \"" + directory + "\"";

	if (std::system(command.c_str()) != 0)
		return false;

	std::string url = repo.url;

	if (!url.empty() && url.back() != '/')
		url += '/';

	if (!download_file(url + "index.csv", path))
		return false;

	std::ifstream file(path);

	if (!file.is_open())
		return false;

	std::string line;

	std::getline(file, line);

	while (std::getline(file, line)) {
		auto fields = split(line, ',');

		if (fields.size() < 2)
			continue;

		std::string name = fields[0];
		std::string meta = fields[1];

		std::string package_directory = directory + "/pkgs/" + name;

		std::string command = "mkdir -p \"" + package_directory + "\"";

		if (std::system(command.c_str()) != 0)
			return false;

		std::string meta_url = url + meta;

		std::string meta_path = package_directory + "/meta.csv";

		if (!download_file(meta_url, meta_path))
			return false;
	}

	return true;
}

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

std::vector<RepositoryConfig> load_repos(const std::string& path) {
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
Repository load_repo(const RepositoryConfig& config) {
	Repository repo;

	repo.name = config.name;
	repo.url = config.url;

	std::string directory = REPOS_DIRECTORY + config.name;

	std::string index_path = directory + "/index.csv";

	std::ifstream file(index_path);

	if (!file.is_open()) {
		printf("failed to open %s\n", index_path.c_str());
		return repo;
	}

	std::string line;

	if (!std::getline(file, line))
		return repo;

	while (std::getline(file, line)) {
		auto fields = split(line, ',');

		if (fields.size() < 2)
			continue;

		Package pkg;

		pkg.name = fields[0];
		pkg.meta = fields[1];

		std::string meta_path = directory + "/pkgs/" + pkg.name + "/meta.csv";

		std::ifstream meta_file(meta_path);

		if (!meta_file.is_open()) {
			printf("failed to open metadata for %s\n", pkg.name.c_str());
			continue;
		}

		std::string meta_line;

		while (std::getline(meta_file, meta_line)) {
			auto meta_fields = split(meta_line, ',');

			if (meta_fields.size() < 2)
				continue;

			if (meta_fields[0] == "description")
				pkg.metadata.description = meta_fields[1];
			else if (meta_fields[0] == "version")
				pkg.metadata.version = meta_fields[1];
			else if (meta_fields[0] == "content")
				pkg.metadata.content = meta_fields[1];
			else if (meta_fields[0] == "depends") {
				if (!meta_fields[1].empty())
					pkg.metadata.depends = split(meta_fields[1], ';');
			}
		}

		repo.packages[pkg.name] = pkg;
	}

	return repo;
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
		else if (fields[0] == "content")
			meta.content = fields[1];
	}

	return meta;
}
