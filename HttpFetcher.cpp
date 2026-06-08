#include "HttpFetcher.h"
#include <curl/curl.h>      // libcurl 库，用于网络请求
#include <iostream>
#include <windows.h>
#include <stdexcept>

// 回调函数：libcurl 每次收到一块数据就会调用它
// 作用是将数据追加到 std::string 中
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t totalSize = size * nmemb;           // 本次收到的字节数
    output->append((char*)contents, totalSize); // 追加到 output 字符串
    return totalSize;                           // 返回已处理的大小，libcurl 据此判断成功
}

// 判断错误类型是否值得重试——只有网络瞬断/超时类错误才重试
static bool isRetryable(CURLcode code) {
    switch (code) {
    case CURLE_COULDNT_RESOLVE_HOST: // DNS 解析失败
    case CURLE_COULDNT_CONNECT:      // 无法建立连接
    case CURLE_OPERATION_TIMEDOUT:   // 超时
    case CURLE_RECV_ERROR:           // 接收数据失败
    case CURLE_SEND_ERROR:           // 发送数据失败
    case CURLE_GOT_NOTHING:          // 服务器无响应
        return true;
    default:
        return false;
    }
}

// 获取指定 URL 的内容，返回响应字符串
// 网络瞬断时自动重试，采用指数退避：1s → 2s → 4s
std::string HttpFetcher::get(const std::string& url) {
    const int maxRetries = 3;   // 最多重试 3 次
    int delayMs = 1000;          // 初始等待 1 秒

    CURL* curl = curl_easy_init();   // 初始化 libcurl 会话
    if (!curl) {
        throw std::runtime_error("curl_easy_init() failed");
    }

    // 设置请求的 URL
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    // 设置 User-Agent，避免被 API 拒绝
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "CryptoTracker/1.0");

    // 显式启用 SSL 证书验证
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

    // 设置连接超时：2 秒内无法建立连接就放弃
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 2L);
    // 设置总超时：5 秒内未完成整个请求就放弃
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);

    // 设置写数据回调函数，libcurl 收到数据时会调用 WriteCallback
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);

    CURLcode res = CURLE_OK;

    for (int attempt = 0; attempt < maxRetries; ++attempt) {
        std::string response;            // 每次重试清空响应
        // 将 response 字符串的地址传给回调函数，让它能写入数据
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        // 执行请求
        res = curl_easy_perform(curl);

        if (res == CURLE_OK) {
            curl_easy_cleanup(curl);   // 清理 libcurl 会话，释放资源
            return response;
        }

        // 已达最大重试次数，或错误不可重试，停止
        if (attempt == maxRetries || !isRetryable(res)) {
            break;
        }

        std::cerr << "请求失败，正在重试 (第 " << (attempt + 1) << " 次): "
                  << curl_easy_strerror(res) << std::endl;
        Sleep(delayMs);       // 指数退避等待
        delayMs *= 2;         // 1s → 2s → 4s
    }

    std::string errorMsg = curl_easy_strerror(res);
    curl_easy_cleanup(curl);   // 清理 libcurl 会话，释放资源
    throw std::runtime_error("HTTP请求失败: " + errorMsg);
}
