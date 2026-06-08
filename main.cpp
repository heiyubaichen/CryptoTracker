#define NOMINMAX
#include "Tracker.h"
#include <iostream>
#include <string>
#include <windows.h>
#include <limits>
#include <fstream>
#include <map>
#include <cctype>

std::map<std::string, std::string> readConfig(const std::string& filename) {
    std::map<std::string, std::string> config;
    std::ifstream file(filename);
    std::string line;
    while (std::getline(file, line)) {
        size_t pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            config[key] = value;
        }
    }
    return config;
}

static bool isValidSymbol(const std::string& symbol) {
    if (symbol.empty() || symbol.size() > 30) return false;
    for (char c : symbol) {
        if (!std::isalnum(static_cast<unsigned char>(c)) && c != '-') return false;
    }
    return true;
}

int main() {
    system("chcp 65001 > nul");
    
    auto config = readConfig("db_config.txt");
    std::string host = config["host"];
    std::string user = config["user"];
    std::string password = config["password"];
    std::string database = config["database"];

    try {
        Tracker tracker(host, user, password, database);

        int choice;
        std::string symbol;

        while (true) {
            std::cout << "\n=== 加密货币行情追踪器 ===" << std::endl;
            std::cout << "1. 更新行情" << std::endl;
            std::cout << "2. 查看历史记录" << std::endl;
            std::cout << "3. 计算移动平均线(SMA)" << std::endl;
            std::cout << "4. 退出" << std::endl;
            std::cout << "请选择: ";
            if (!(std::cin >> choice)) {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "输入无效，请输入数字。" << std::endl;
                continue;
            }

            switch (choice) {
            case 1: {
                std::cout << "输入币种ID (例如: bitcoin, ethereum, dogecoin): ";
                std::cin >> symbol;
                tracker.updatePrice(symbol);
                break;
            }
            case 2: {
                std::cout << "输入币种ID (例如: bitcoin, ethereum, dogecoin): ";
                std::cin >> symbol;
                std::cout << "查询天数 (例如: 7): ";
                int days;
                if (!(std::cin >> days)) {
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    std::cout << "输入无效，请输入数字。" << std::endl;
                    break;
                }
                tracker.showHistory(symbol, days);
                break;
            }
            case 3: {
                std::cout << "输入币种ID (例如: bitcoin, ethereum, dogecoin): ";
                std::cin >> symbol;
                std::cout << "计算天数 (例如: 5): ";
                int period;
                if (!(std::cin >> period)) {
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    std::cout << "输入无效，请输入数字。" << std::endl;
                    break;
                }
                tracker.showSMA(symbol, period);
                break;
            }
            case 4: {
                std::cout << "再见！" << std::endl;
                return 0;
            }
            default: {
                std::cout << "无效选择，请重试。" << std::endl;
            }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "启动失败: " << e.what() << std::endl;
        return 1;
    }
}