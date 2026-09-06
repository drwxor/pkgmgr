#include "package.hpp"
#include "csv.hpp"

#include <fstream>
#include <vector>

Package* find_package(
	std::vector<Package>& packages,
	const std::string& name
)
{
	for (auto& pkg : packages)
	{
		if (pkg.name == name)
			return &pkg;
	}

	return nullptr;
}

std::vector<Package> load_repo(const std::string& path)
{
	std::ifstream file(path);

	if (!file.is_open())
	{
		printf("failed to open %s\n", path.c_str());
		return {};
	}

	std::vector<Package> packages;
	std::string line;

	std::getline(file, line);

	while (std::getline(file, line))
	{
		auto fields = split(line, ',');

		if (fields.size() < 3)
		{
			printf("bad csv line\n");
			continue;
		}

		Package pkg;

		pkg.name = fields[0];
		pkg.version = fields[1];
		pkg.url = fields[2];

		packages.push_back(pkg);
	}

	return packages;
}
