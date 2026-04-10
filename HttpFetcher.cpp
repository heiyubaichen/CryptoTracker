#include "HttpFetcher.h"
#include <curl/curl.h>      // libcurl 库，用于网络请求
#include <iostream>         // 输出错误信息

// 回调函数：libcurl 每次收到一块数据就会调用它
// 作用是将数据追加到 std::string 中
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t totalSize = size * nmemb;           // 本次收到的字节数
    output->append((char*)contents, totalSize); // 追加到 output 字符串
    return totalSize;                           // 返回已处理的大小，libcurl 据此判断成功
}

std::string HttpFetcher::get(const std::string& url) {
    CURL* curl = curl_easy_init();   // 初始化 libcurl 会话
    std::string response;            // 存储响应内容

    if (curl) {
        // 设置请求的 URL
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        // 设置连接超时：10 秒内无法建立连接就放弃
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
        // 设置总超时：15 秒内未完成整个请求就放弃
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);

        // 设置写数据回调函数，libcurl 收到数据时会调用 WriteCallback
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        // 将 response 字符串的地址传给回调函数，让它能写入数据
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        // 执行请求
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            // 请求失败，打印错误信息
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }
        // 清理 libcurl 会话，释放资源
        curl_easy_cleanup(curl);
    }
    return response;   // 返回响应字符串（可能为空）
}