#include "JsonParser.h"
#include <nlohmann/json.hpp>   // 第三方 JSON 库，非常易用
#include <string>

using json = nlohmann::json;   // 简化写法

double JsonParser::parsePrice(const std::string& jsonStr, const std::string& coinId) {
    // 解析 JSON 字符串为 json 对象
    json data = json::parse(jsonStr);
    // Coinlore API 返回一个数组，第一个元素就是币种信息
    if (data.is_array() && !data.empty()) {
        // price_usd 字段是字符串，需要转换成 double
        return std::stod(data[0]["price_usd"].get<std::string>());
    }
    return 0.0;   // 解析失败返回 0
}

double JsonParser::parse24hChange(const std::string& jsonStr, const std::string& coinId) {
    json data = json::parse(jsonStr);
    if (data.is_array() && !data.empty()) {
        // percent_change_24h 也是字符串
        return std::stod(data[0]["percent_change_24h"].get<std::string>());
    }
    return 0.0;
}