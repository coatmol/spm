#pragma once

#include <string>

namespace spm
{
    class Extractor
    {
    public:
        static bool extractZip(const std::string &zipFilePath, const std::string &destinationDir);
    };
}