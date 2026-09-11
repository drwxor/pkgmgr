#include "package.hpp"

#include <cstdio>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

#include "extract.hpp"
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

	if (!input.is_open()) {
        printf("unable to open %s\n", INSTALLED_PATH.c_str());
		return false;
    }

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

		if (std::remove(fields[2].c_str()) != 0) {
		    printf("unable to remove %s\n", fields[2].c_str());
			return false;
	    }
	}

	input.close();

	std::ofstream output(INSTALLED_PATH);

	if (!output.is_open()) {
	    printf("unable to open %s\n", INSTALLED_PATH.c_str());
		return false;
	}

	for (auto& entry : entries)
		output << entry << '\n';

	return true;
}

bool install_package_from_repo(Repository* repo, const Package* pkg) {
    std::string output = pkg->name + ".tar.zst";

	if (!download_package(*repo, *pkg, output)) {
		printf("download failed\n");
		return false;
	}

	std::ifstream installed(INSTALLED_PATH);

	if (!installed.good()) {
		std::ofstream file(INSTALLED_PATH);

		if (!file.is_open()) {
			printf("failed to create installed database\n");
			return false;
		}

		file << "name,version,path\n";
	}

	std::vector<std::string> files;

	if (!extract_package(output, PACKAGE_EXTRACT_DIRECTORY, files)) {
		printf("extraction failed\n");
		std::remove(output.c_str());
		return false;
	}

	std::ofstream file(INSTALLED_PATH, std::ios::app);

	if (!file.is_open()) {
		printf("failed to open installed database\n");
		return false;
	}

	for (auto& path : files) {
		file << pkg->name << "," << pkg->metadata.version << "," << path << "\n";
	}

	std::remove(output.c_str());

	printf("installed %s\n", pkg->name.c_str());

	return true;
}

bool install_package(const char *name, std::vector<RepositoryConfig>& repos) {
    if (is_installed(name)) {
		printf("%s is already installed\n", name);
		return false;
	}

	bool found = false;

	for (auto& config : repos) {
		auto repo = load_repo(config);
		Package* pkg = find_package(repo, name);

		if (!pkg)
			continue;

		found = true;

		bool dependency_failed = false;

		for (auto& dep : pkg->metadata.depends) {
			printf("installing dependency %s\n", dep.c_str());

			if (!install_package(dep.c_str(), repos)) {
				printf("failed to install %s\n", dep.c_str());
				dependency_failed = true;
				break;
			}
		}

		if (dependency_failed)
			return false;

		if (!install_package_from_repo(&repo, pkg)) {
		    printf("failed to install %s", name);
		    return false;
		}

		break;
	}

	if (!found) {
	    printf("%s: package not found\n", name);
		return false;
	}

	return true;
}
