#pragma once

#include <string>

namespace spm {
    class Downloader
    {
    public:
        static bool downloadText(const std::string& host, const std::string& path, std::string& outResponse);
        static bool downloadFile(const std::string& host, const std::string& path, const std::string& outputPath);
    };
}