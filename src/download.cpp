#include "download.hpp"

#include <cstdio>

#include <curl/curl.h>

static size_t write_data(
	void* ptr,
	size_t size,
	size_t nmemb,
	void* stream
)
{
	FILE* file = static_cast<FILE*>(stream);

	return std::fwrite(ptr, size, nmemb, file);
}

bool download_file(
	const std::string& url,
	const std::string& output
)
{
	CURL* curl = curl_easy_init();

	if (!curl)
		return false;

	FILE* file = std::fopen(output.c_str(), "wb");

	if (!file)
	{
		curl_easy_cleanup(curl);
		return false;
	}

	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

	CURLcode result = curl_easy_perform(curl);

	if (result != CURLE_OK)
	{
		printf("curl: %s\n", curl_easy_strerror(result));

		std::fclose(file);
		curl_easy_cleanup(curl);

		return false;
	}

	long response_code = 0;

	curl_easy_getinfo(
		curl,
		CURLINFO_RESPONSE_CODE,
		&response_code
	);

	if (response_code < 200 || response_code >= 300)
	{
		std::fclose(file);
		curl_easy_cleanup(curl);
		return false;
	}

	std::fclose(file);
	curl_easy_cleanup(curl);

	return result == CURLE_OK;
}
