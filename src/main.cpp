#include <string>

#include <curl/curl.h>

#include "repo.hpp"
#include "download.hpp"

enum Command {
    COMMAND_INSTALL,
    COMMAND_INFO,
    COMMAND_UNKNOWN,
};

Command parse_command(int argc, char** argv) {
	if (argc < 2)
		return COMMAND_UNKNOWN;

	std::string cmd = argv[1];

	if (cmd == "install")
		return COMMAND_INSTALL;

	if (cmd == "info")
		return COMMAND_INFO;

	return COMMAND_UNKNOWN;
}

void print_help() {
    printf("pkgmgr <command>\n");
    printf("\tinstall <package>\n");
    printf("\tinfo <package>\n");
    printf("\tremove <package>\n");
}

int main(int argc, char** argv) {
    enum Command cmd = parse_command(argc, argv);

    auto packages = load_repo("testrepo/index.csv");

    switch (cmd) {
        case COMMAND_UNKNOWN: {
            print_help();
            return 1;
        }

        case COMMAND_INSTALL: {
            if (argc < 3)
    		{
    			printf("missing package name\n");
    			return 1;
    		}

            curl_global_init(CURL_GLOBAL_DEFAULT);

    		auto pkg = find_package(packages, argv[2]);

    		if (!pkg)
    		{
                curl_global_cleanup();
    			printf("package not found\n");
    			return 1;
    		}

            if (!download_file(pkg->url, pkg->name + ".pkg"))
            {
                printf("download failed\n");
                return 1;
            }

            printf("downloaded %s\n", pkg->name.c_str());

            curl_global_cleanup();
            break;
        }

        case COMMAND_INFO: {
            if (argc < 3) {
    			printf("missing package name\n");
    			return 1;
    		}

    		auto pkg = find_package(packages, argv[2]);

    		if (!pkg) {
    			printf("package not found\n");
    			return 1;
    		}

    		printf("name: %s\n", pkg->name.c_str());
    		printf("version: %s\n", pkg->version.c_str());
    		printf("url: %s\n", pkg->url.c_str());

            break;
        }
    }

    return 0;
}
