#include <cstdio>
#include <string>
#include <vector>
#include <fstream>

#include <curl/curl.h>

#include "config.hpp"
#include "repo.hpp"
#include "download.hpp"
#include "extract.hpp"
#include "package.hpp"
#include "models.hpp"

enum Command {
	COMMAND_SYNC,
	COMMAND_INSTALL,
	COMMAND_REMOVE,
	COMMAND_INFO,
	COMMAND_UNKNOWN,
};

Command parse_command(int argc, char** argv) {
	if (argc < 2)
		return COMMAND_UNKNOWN;

	std::string cmd = argv[1];

	if (cmd == "sync")
		return COMMAND_SYNC;

	if (cmd == "add")
		return COMMAND_INSTALL;

	if (cmd == "del")
		return COMMAND_REMOVE;

	if (cmd == "info")
		return COMMAND_INFO;

	return COMMAND_UNKNOWN;
}

void print_help() {
	printf("pkgmgr <command>\n");
	printf("\tsync\n");
	printf("\tadd <package>\n");
	printf("\tdel <package>\n");
	printf("\tinfo <package>\n");
}

int main(int argc, char** argv) {
	Command cmd = parse_command(argc, argv);

	curl_global_init(CURL_GLOBAL_DEFAULT);

	switch (cmd) {
		case COMMAND_UNKNOWN: {
			print_help();
			curl_global_cleanup();
			return 1;
		}

		case COMMAND_SYNC: {
			auto repos = load_repos(REPOS_PATH);

			if (repos.empty()) {
				printf("no repositories configured\n");
				curl_global_cleanup();
				return 1;
			}

			for (auto& repo : repos) {
				printf("syncing %s...\n", repo.name.c_str());

				if (!sync_repo(repo)) {
					printf("failed to sync %s\n", repo.name.c_str());

					curl_global_cleanup();
					return 1;
				}

				printf("synchronized %s\n", repo.name.c_str());
			}

			curl_global_cleanup();
			return 0;
		}

		case COMMAND_INSTALL: {
			if (argc < 3) {
				printf("missing package name\n");
				curl_global_cleanup();
				return 1;
			}

			auto repos = load_repos(REPOS_PATH);

			for (auto& config : repos) {
				auto repo = load_repo(config);

				auto pkg = find_package(repo, argv[2]);

				if (!pkg)
					continue;

				std::string output = pkg->name + ".tar.zst";

				if (!download_package(repo, *pkg, output)) {
					printf("download failed\n");
					return 1;
				}

				std::ifstream installed(INSTALLED_PATH);

				if (!installed.good()) {
					std::ofstream file(INSTALLED_PATH);

					if (!file.is_open()) {
						printf("failed to create installed database\n");
						return 1;
					}

					file << "name,version,path\n";
				}

				std::vector<std::string> files;

				if (!extract_package(output, PACKAGE_EXTRACT_DIRECTORY, files)) {
					printf("extraction failed\n");
					std::remove(output.c_str());
					return 1;
				}

				std::ofstream file(INSTALLED_PATH, std::ios::app);

				if (!file.is_open()) {
					printf("failed to open installed database\n");
					return 1;
				}

				for (auto& path : files) {
					file << pkg->name << "," << pkg->metadata.version << "," << path << "\n";
				}

				std::remove(output.c_str());

				printf("installed %s\n", pkg->name.c_str());

				curl_global_cleanup();
				return 0;
			}

			printf("package not found\n");

			curl_global_cleanup();
			return 1;
		}

		case COMMAND_REMOVE: {
			if (argc < 3) {
				printf("missing package name\n");
				return 1;
			}

			if (!remove_package(argv[2])) {
				printf("failed to remove package\n");
				return 1;
			}

			printf("removed %s\n", argv[2]);
			break;
		}

		case COMMAND_INFO: {
			if (argc < 3) {
				printf("missing package name\n");
				curl_global_cleanup();
				return 1;
			}

			auto repos = load_repos(REPOS_PATH);

			for (auto& config : repos) {
				auto repo = load_repo(config);

				auto pkg = find_package(repo, argv[2]);

				if (!pkg)
					continue;

				printf("name: %s\n", pkg->name.c_str());
				printf("version: %s\n", pkg->metadata.version.c_str());
				printf("description: %s\n", pkg->metadata.description.c_str());

				printf("depends:");

				for (auto& dep : pkg->metadata.depends)
					printf(" %s", dep.c_str());

				printf("\n");

				curl_global_cleanup();
				return 0;
			}

			printf("package not found\n");

			curl_global_cleanup();
			return 1;
		}
	}

	curl_global_cleanup();
	return 0;
}
