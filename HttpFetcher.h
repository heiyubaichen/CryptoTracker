#pragma once
#include <string>

class HttpFetcher {
public:
    std::string get(const std::string& url);
};