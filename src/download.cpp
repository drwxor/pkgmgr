#include "download.hpp"

#include <cstdio>
#include <fstream>
#include <string>
#include <sstream>

#include <curl/curl.h>

static size_t write_file(void* ptr, size_t size, size_t nmemb, void* stream) {
	FILE* file = static_cast<FILE*>(stream);

	return std::fwrite(ptr, size, nmemb, file);
}

static size_t write_string(void* ptr, size_t size, size_t nmemb, void* stream) {
	std::string* output = static_cast<std::string*>(stream);

	output->append(static_cast<char*>(ptr), size * nmemb);

	return size * nmemb;
}

bool download_file(const std::string& url, const std::string& output) {
	CURL* curl = curl_easy_init();

	if (!curl)
		return false;

	FILE* file = std::fopen(output.c_str(), "wb");

	if (!file) {
		curl_easy_cleanup(curl);
		return false;
	}

	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_file);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

	CURLcode result = curl_easy_perform(curl);

	long response_code = 0;

	if (result == CURLE_OK) {
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
	}

	std::fclose(file);
	curl_easy_cleanup(curl);

	if (result != CURLE_OK) {
		printf("curl: %s\n", curl_easy_strerror(result));
		std::remove(output.c_str());
		return false;
	}

	if (response_code < 200 || response_code >= 300) {
		printf("HTTP: %ld\n", response_code);
		std::remove(output.c_str());
		return false;
	}

	return true;
}

bool download_string(const std::string& url, std::string& output) {
	CURL* curl = curl_easy_init();

	if (!curl)
		return false;

	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_string);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &output);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

	CURLcode result = curl_easy_perform(curl);

	long response_code = 0;

	if (result == CURLE_OK) {
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
	}

	curl_easy_cleanup(curl);

	if (result != CURLE_OK) {
		printf("curl: %s\n", curl_easy_strerror(result));
		return false;
	}

	if (response_code < 200 || response_code >= 300){
		printf("HTTP: %ld\n", response_code);
		return false;
	}

	return true;
}
