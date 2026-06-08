#include "Tracker.h"
#include "JsonParser.h"
#include <iostream>
#include <iomanip>
#include <numeric>
#include <map>
#include <stdexcept>

namespace {
    const std::map<std::string, std::string> coinIdMap = {
        {"bitcoin", "90"},
        {"ethereum", "80"},
        {"dogecoin", "2"}
    };
}

Tracker::Tracker(const std::string& host, const std::string& user,
    const std::string& pass, const std::string& db) {
    dbManager.connect(host, user, pass, db);
}

bool Tracker::isValidCoin(const std::string& symbol) const {
    return coinIdMap.find(symbol) != coinIdMap.end();
}

bool Tracker::updatePrice(const std::string& symbol) {
    if (!isValidCoin(symbol)) {
        std::cerr << "不支持的币种: " << symbol << std::endl;
        return false;
    }
    std::string url = "https://api.coinlore.net/api/ticker/?id=" + coinIdMap.at(symbol);

    std::string response;
    try {
        response = httpFetcher.get(url);
    }
    catch (const std::exception& e) {
        std::cerr << "网络请求失败: " << e.what() << std::endl;
        return false;
    }

    double price = 0.0, change = 0.0;
    try {
        price = JsonParser::parsePrice(response, symbol);
        change = JsonParser::parse24hChange(response, symbol);
    }
    catch (const std::exception& e) {
        std::cerr << "数据解析失败: " << e.what() << std::endl;
        return false;
    }

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "当前价格: $" << price << std::endl;
    std::cout << "24h涨跌幅: " << std::showpos << change << "%" << std::noshowpos << std::endl;

    try {
        dbManager.insertPrice(symbol, price, change);
    }
    catch (const std::exception& e) {
        std::cerr << "数据库操作失败: " << e.what() << std::endl;
        return false;
    }
    return true;
}

void Tracker::showHistory(const std::string& symbol, int days) {
    if (!isValidCoin(symbol)) {
        std::cerr << "不支持的币种: " << symbol << std::endl;
        return;
    }
    try {
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
    catch (const std::exception& e) {
        std::cerr << "数据库操作失败: " << e.what() << std::endl;
        return;
    }
    return;
}

double Tracker::calculateSMA(const std::vector<double>& prices) {
    if (prices.empty()) return 0.0;
    double sum = std::accumulate(prices.begin(), prices.end(), 0.0);
    return sum / prices.size();
}

void Tracker::showSMA(const std::string& symbol, int period) {
    if (!isValidCoin(symbol)) {
        std::cerr << "不支持的币种: " << symbol << std::endl;
        return;
    }
    try {
        auto records = dbManager.getRecentPrices(symbol, period);
        if (records.empty()) {
            std::cout << "未找到 " << symbol << " 的历史数据。" << std::endl;
            return;
        }

        std::map<std::string, std::vector<double>> dailyPrices;
        for (const auto& rec : records) {
            std::string date = rec.created_at.substr(0, 10); // 提取日期部分
            dailyPrices[date].push_back(rec.price);
        }

        //将同一天的价格取平均，得到每天的平均价
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
    catch (const std::exception& e) {
        std::cerr << "数据库操作失败: " << e.what() << std::endl;
    }
    return;
}