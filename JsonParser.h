#pragma once
#include <string>

class JsonParser {
public:
    static double parsePrice(const std::string& jsonStr, const std::string& coinId);
    static double parse24hChange(const std::string& jsonStr, const std::string& coinId);
};