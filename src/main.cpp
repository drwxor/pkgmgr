#include <cstdio>
#include <string>
#include <vector>

#include <curl/curl.h>

#include "config.hpp"
#include "repo.hpp"
#include "download.hpp"

enum Command {
	COMMAND_SYNC,
	COMMAND_INSTALL,
	COMMAND_INFO,
	COMMAND_UNKNOWN,
};

Command parse_command(int argc, char** argv) {
	if (argc < 2)
		return COMMAND_UNKNOWN;

	std::string cmd = argv[1];

	if (cmd == "sync")
		return COMMAND_SYNC;

	if (cmd == "install")
		return COMMAND_INSTALL;

	if (cmd == "info")
		return COMMAND_INFO;

	return COMMAND_UNKNOWN;
}

void print_help() {
	printf("pkgmgr <command>\n");
	printf("\tsync\n");
	printf("\tinstall <package>\n");
	printf("\tinfo <package>\n");
}

int main(int argc, char** argv) {
	Command cmd = parse_command(argc, argv);

	curl_global_init(CURL_GLOBAL_DEFAULT);

	switch (cmd)
	{
		case COMMAND_UNKNOWN: {
			print_help();
			curl_global_cleanup();
			return 1;
		}

		case COMMAND_SYNC: {
			auto repos = load_repositories(REPOS_PATH);

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

			auto repos = load_repositories(REPOS_PATH);

			for (auto& config : repos) {
				auto repo = load_repo(config);

				auto pkg = find_package(
					repo,
					argv[2]
				);

				if (!pkg)
					continue;

				printf("found %s in %s\n", pkg->name.c_str(), repo.name.c_str());

				printf("meta: %s\n", pkg->meta.c_str());

				curl_global_cleanup();
				return 0;
			}

			printf("package not found\n");

			curl_global_cleanup();
			return 1;
		}

		case COMMAND_INFO: {
			if (argc < 3) {
				printf("missing package name\n");
				curl_global_cleanup();
				return 1;
			}

			auto repos = load_repositories(REPOS_PATH);

			for (auto& config : repos) {
				auto repo = load_repo(config);

				auto pkg = find_package(repo,argv[2]);

				if (!pkg)
					continue;

				auto meta = load_package_meta(repo, *pkg);

				printf("name: %s\n", pkg->name.c_str());
				printf("version: %s\n", meta.version.c_str());
				printf("description: %s\n", meta.description.c_str());

				printf("depends:");

				for (auto& dep : meta.depends)
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
