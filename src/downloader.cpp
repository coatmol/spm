#include "downloader.hpp"
#include <windows.h>
#include <wininet.h>
#include <fstream>
#include <iostream>

#pragma comment(lib, "wininet.lib")

namespace spm {

bool Downloader::downloadText(const std::string& hostUrl, const std::string& path, std::string& outResponse) {
    // Strip "https://" or "http://" prefix if passed in the host string
    std::string cleanHost = hostUrl;
    if (cleanHost.rfind("https://", 0) == 0) cleanHost = cleanHost.substr(8);
    else if (cleanHost.rfind("http://", 0) == 0) cleanHost = cleanHost.substr(7);

    HINTERNET hInternet = InternetOpenA("spm-agent", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hInternet) return false;

    HINTERNET hConnect = InternetConnectA(hInternet, cleanHost.c_str(), INTERNET_DEFAULT_HTTPS_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
    if (!hConnect) {
        InternetCloseHandle(hInternet);
        return false;
    }

    HINTERNET hRequest = HttpOpenRequestA(hConnect, "GET", path.c_str(), NULL, NULL, NULL, INTERNET_FLAG_SECURE | INTERNET_FLAG_RELOAD, 0);
    if (!hRequest) {
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        return false;
    }

    if (HttpSendRequestA(hRequest, NULL, 0, NULL, 0)) {
        char buffer[4096];
        DWORD bytesRead = 0;
        while (InternetReadFile(hRequest, buffer, sizeof(buffer) - 1, &bytesRead) && bytesRead > 0) {
            buffer[bytesRead] = '\0';
            outResponse += buffer;
        }
    }

    InternetCloseHandle(hRequest);
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);
    return !outResponse.empty();
}

bool Downloader::downloadFile(const std::string& hostUrl, const std::string& path, const std::string& outputPath) {
    std::string cleanHost = hostUrl;
    if (cleanHost.rfind("https://", 0) == 0) cleanHost = cleanHost.substr(8);
    else if (cleanHost.rfind("http://", 0) == 0) cleanHost = cleanHost.substr(7);

    HINTERNET hInternet = InternetOpenA("spm-agent", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hInternet) return false;

    HINTERNET hConnect = InternetConnectA(hInternet, cleanHost.c_str(), INTERNET_DEFAULT_HTTPS_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
    if (!hConnect) {
        InternetCloseHandle(hInternet);
        return false;
    }

    HINTERNET hRequest = HttpOpenRequestA(hConnect, "GET", path.c_str(), NULL, NULL, NULL, INTERNET_FLAG_SECURE | INTERNET_FLAG_RELOAD, 0);
    if (!hRequest) {
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        return false;
    }

    if (HttpSendRequestA(hRequest, NULL, 0, NULL, 0)) {
        std::ofstream file(outputPath, std::ios::binary);
        if (!file.is_open()) return false;

        char buffer[8192];
        DWORD bytesRead = 0;
        while (InternetReadFile(hRequest, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
            file.write(buffer, bytesRead);
        }
        std::cout << "[spm] Successfully downloaded to " << outputPath << "\n";
        
        InternetCloseHandle(hRequest);
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        return true;
    }

    InternetCloseHandle(hRequest);
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);
    return false;
}

}