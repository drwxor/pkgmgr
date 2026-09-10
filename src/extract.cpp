#include "extract.hpp"

#include <cstdio>
#include <vector>

#include <archive.h>
#include <archive_entry.h>

bool extract_package(const std::string& archive, const std::string& destination, std::vector<std::string>& files) {
	struct archive* input = archive_read_new();

	archive_read_support_filter_zstd(input);
	archive_read_support_format_tar(input);

	if (archive_read_open_filename(input, archive.c_str(), 10240) != ARCHIVE_OK) {
		printf("failed to open archive: %s\n", archive_error_string(input));

		archive_read_free(input);
		return false;
	}

	struct archive* output = archive_write_disk_new();

	archive_write_disk_set_options(output, ARCHIVE_EXTRACT_TIME | ARCHIVE_EXTRACT_PERM | ARCHIVE_EXTRACT_ACL | ARCHIVE_EXTRACT_FFLAGS);
	archive_write_disk_set_standard_lookup(output);

	struct archive_entry* entry;

	while (true) {
		int result = archive_read_next_header(input, &entry);

		if (result == ARCHIVE_EOF)
			break;

		if (result != ARCHIVE_OK) {
			printf("failed to read archive: %s\n", archive_error_string(input));

			archive_write_free(output);
			archive_read_free(input);
			return false;
		}

		const char* name = archive_entry_pathname(entry);

		std::string path = destination + "/" + name;

		archive_entry_set_pathname(entry, path.c_str());

		result = archive_write_header(output, entry);

		if (result != ARCHIVE_OK) {
			printf("failed to write %s: %s\n", path.c_str(), archive_error_string(output));

			archive_write_free(output);
			archive_read_free(input);
			return false;
		}

		if (archive_entry_size(entry) > 0) {
			const void* buffer;
			size_t size;
			la_int64_t offset;

			while (true) {
				result = archive_read_data_block(input, &buffer, &size, &offset);

				if (result == ARCHIVE_EOF)
					break;

				if (result != ARCHIVE_OK) {
					printf("failed to read archive data: %s\n", archive_error_string(input));

					archive_write_free(output);
					archive_read_free(input);
					return false;
				}

				result = archive_write_data_block(output, buffer, size, offset);

				if (result != ARCHIVE_OK) {
					printf("failed to write archive data: %s\n", archive_error_string(output));

					archive_write_free(output);
					archive_read_free(input);
					return false;
				}

				files.push_back(path);
			}
		}

		result = archive_write_finish_entry(output);

		if (result != ARCHIVE_OK) {
			printf("failed to finish entry: %s\n", archive_error_string(output));

			archive_write_free(output);
			archive_read_free(input);
			return false;
		}
	}

	archive_write_free(output);
	archive_read_free(input);

	return true;
}
