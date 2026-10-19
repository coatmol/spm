#pragma once
#include <string>
#include <sqlite3.h>

namespace spm
{
    class Database
    {
    public:
        explicit Database(const std::string &dbPath);
        ~Database();

        // Prevent copying
        Database(const Database &) = delete;
        Database &operator=(const Database &) = delete;

        bool initializeSchema();
        bool registerPackage(const std::string &name, const std::string &version, const std::string &installPath);

        bool isPackageRegistered(const std::string &name);
        bool isPackageRegistered(const std::string &name, const std::string &version);

    private:
        sqlite3 *db_ = nullptr;
    };
}