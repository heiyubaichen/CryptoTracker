#include "JsonParser.h"
#include <nlohmann/json.hpp>
#include <string>
#include <stdexcept>

using json = nlohmann::json;

double JsonParser::parsePrice(const std::string& jsonStr, const std::string& coinId) {
    if (jsonStr.empty()) {
        throw std::runtime_error("JSON parse error: empty input.");
    }
    json data = json::parse(jsonStr);
    if (!data.is_array() || data.empty()) {
        throw std::runtime_error("JSON parse error: unexpected response format.");
    }
    return std::stod(data[0]["price_usd"].get<std::string>());
}

double JsonParser::parse24hChange(const std::string& jsonStr, const std::string& coinId) {
    if (jsonStr.empty()) {
        throw std::runtime_error("JSON parse error: empty input.");
    }
    json data = json::parse(jsonStr);
    if (!data.is_array() || data.empty()) {
        throw std::runtime_error("JSON parse error: unexpected response format.");
    }
    return std::stod(data[0]["percent_change_24h"].get<std::string>());
}