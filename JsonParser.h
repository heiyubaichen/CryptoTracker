#pragma once
#include <string>

class JsonParser {
public:
    // 从 JSON 字符串中提取价格（美元）
    static double parsePrice(const std::string& jsonStr, const std::string& coinId);
    // 从 JSON 字符串中提取 24 小时涨跌幅（百分比）
    static double parse24hChange(const std::string& jsonStr, const std::string& coinId);
};