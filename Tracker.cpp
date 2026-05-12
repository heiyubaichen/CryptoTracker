#include "Tracker.h"
#include "JsonParser.h"
#include <iostream>
#include <iomanip>
#include <numeric>
#include <map>

Tracker::Tracker(const std::string& host, const std::string& user,
    const std::string& pass, const std::string& db) {
    isConnected = dbManager.connect(host, user, pass, db);
}

bool Tracker::updatePrice(const std::string& symbol) {
    if (!isConnected) {
        std::cerr << "数据库未连接！" << std::endl;
        return false;
    }

    // 币种名称到 Coinlore ID 的映射
    std::map<std::string, std::string> coinIdMap = {
        {"bitcoin", "90"},
        {"ethereum", "80"},
        {"dogecoin", "2"}
    };
    if (coinIdMap.find(symbol) == coinIdMap.end()) {
        std::cerr << "不支持的币种: " << symbol << std::endl;
        return false;
    }
    std::string url = "https://api.coinlore.net/api/ticker/?id=" + coinIdMap[symbol];

    std::string response = httpFetcher.get(url);
    //std::cout << "response: " << response << std::endl;
    if (response.empty()) {
        std::cerr << "获取数据失败！" << std::endl;
        return false;
    }

    double price = JsonParser::parsePrice(response, symbol);
    double change = JsonParser::parse24hChange(response, symbol);

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "当前价格: $" << price << std::endl;
    std::cout << "24h涨跌幅: " << std::showpos << change << "%" << std::noshowpos << std::endl;

    return dbManager.insertPrice(symbol, price, change);
}

void Tracker::showHistory(const std::string& symbol, int days) {
    if (!isConnected) {
        std::cerr << "数据库未连接！" << std::endl;
        return;
    }
    auto records = dbManager.getRecentPrices(symbol, days);
    if (records.empty()) {
        std::cout << "未找到 " << symbol << " 的历史数据。" << std::endl;
        return;
    }
    std::cout << std::left << std::setw(20) << "时间" << std::setw(12) << "价格(USD)" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    for (const auto& rec : records) {
        std::cout << std::left << std::setw(20) << rec.created_at
            << "$" << std::setw(10) << std::fixed << std::setprecision(2)
            << rec.price << std::endl;
    }
}

double Tracker::calculateSMA(const std::vector<double>& prices) {
    if (prices.empty()) return 0.0;
    double sum = std::accumulate(prices.begin(), prices.end(), 0.0);
    return sum / prices.size();
}

void Tracker::showSMA(const std::string& symbol, int period) {
    if (!isConnected) {
        std::cerr << "数据库未连接！" << std::endl;
        return;
    }
    auto records = dbManager.getRecentPrices(symbol, period);
    if (records.empty()) {
        std::cout << "未找到 " << symbol << " 的历史数据。" << std::endl;
        return;
    }

    // 按天分组，计算每日均价，避免采样不均导致 SMA 失真
    std::map<std::string, std::vector<double>> dailyPrices;
    for (const auto& rec : records) {
        std::string date = rec.created_at.substr(0, 10); // "YYYY-MM-DD"
        dailyPrices[date].push_back(rec.price);
    }

    std::vector<double> dailyAvgs;
    for (const auto& entry : dailyPrices) {
        const auto& prices = entry.second;
        double sum = std::accumulate(prices.begin(), prices.end(), 0.0);
        dailyAvgs.push_back(sum / prices.size());
    }

    int actualDays = dailyAvgs.size();
    double sma = calculateSMA(dailyAvgs);
    double lastPrice = records.back().price;

    std::cout << std::fixed << std::setprecision(2);
    std::cout << period << "日均线 (基于日均价): $" << sma << std::endl;
    std::cout << "覆盖交易日: " << actualDays << " 天";
    if (actualDays < period) {
        std::cout << " (数据不足" << period << "天，结果仅供参考)";
    }
    std::cout << std::endl;
    std::cout << "最新价格: $" << lastPrice << std::endl;

    if (lastPrice > sma) {
        std::cout << "当前价格高于均线，处于上升趋势。" << std::endl;
    }
    else if (lastPrice < sma) {
        std::cout << "当前价格低于均线，处于下降趋势。" << std::endl;
    }
    else {
        std::cout << "价格与均线持平。" << std::endl;
    }
}