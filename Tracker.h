#pragma once
#include "DatabaseManager.h"
#include "HttpFetcher.h"
#include <string>
#include <vector>

class Tracker {
public:
    // 构造函数：连接数据库
    Tracker(const std::string& host, const std::string& user,
        const std::string& pass, const std::string& db);
    // 更新指定币种的行情（获取 API、存入数据库）
    bool updatePrice(const std::string& symbol);
    // 显示历史价格
    void showHistory(const std::string& symbol, int days);
    // 显示简单移动平均线（SMA）
    void showSMA(const std::string& symbol, int period);

    // 检查币种是否受支持
    bool isValidCoin(const std::string& symbol) const;

private:
    DatabaseManager dbManager;
    HttpFetcher httpFetcher;

    double calculateSMA(const std::vector<double>& prices);
};