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
	COMMAND_PURGE,
	COMMAND_INFO,
	COMMAND_LIST,
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

	if (cmd == "purge")
		return COMMAND_PURGE;

	if (cmd == "info")
		return COMMAND_INFO;

	if (cmd == "list")
		return COMMAND_LIST;

	return COMMAND_UNKNOWN;
}

void print_help() {
	printf("pkgmgr <command>\n");
	printf("\tsync\n");
	printf("\tadd <packages>\n");
	printf("\tdel <packages>\n");
	printf("\tpurge <packages>\n");
	printf("\tinfo <package>\n");
	printf("\tlist\n");
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

		case COMMAND_LIST: {
		    auto installed = get_installed();

			for (Package& pkg : installed) {
			    printf("%s %s\n", pkg.name.c_str(), pkg.metadata.version.c_str());
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

			for (int i = 2; i < argc; i++) {
			    const char *name = argv[i];
                install_package(name, repos);
			}

			curl_global_cleanup();
			return 1;
		}

		case COMMAND_REMOVE: {
			if (argc < 3) {
				printf("missing package name\n");
				return 1;
			}

			for (int i = 2; i < argc; i++) {
                const char* name = argv[i];

                if (!is_installed(name)) {
					printf("%s is not installed\n", name);
					continue;
				}

                if (!remove_package(name)) {
    				printf("failed to remove %s\n", name);
    				continue;
    			}

    			printf("removed %s\n", name);
			}

			break;
		}

		case COMMAND_PURGE: {
			if (argc < 3) {
				printf("missing package name\n");
				return 1;
			}

			auto repos = load_repos(REPOS_PATH);

			for (int i = 2; i < argc; i++) {
                const char* pkg_name = argv[i];

                for (auto& config : repos) {
    				auto repo = load_repo(config);

    				auto pkg = find_package(repo, argv[2]);

    				if (!pkg)
    					continue;

                    for (auto& name : pkg->metadata.depends) {
                        printf("removing dependency %s\n", name.c_str());

                        if (!is_installed(name)) {
                            printf("%s is not installed\n", name.c_str());
                            continue;
                        }

                        if (!remove_package(name)) {
                            printf("failed to remove %s\n", name.c_str());
                            continue;
                        }

                        printf("removed dependency %s\n", name.c_str());
                    }
                }

                if (!is_installed(pkg_name)) {
                    printf("%s is not installed\n", pkg_name);
                    continue;
                }

                if (!remove_package(pkg_name)) {
                    printf("failed to remove %s\n", pkg_name);
                    continue;
                }

                printf("removed %s\n", pkg_name);
			}

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
