#pragma once
#include <string>
#include <sqlite3.h>
#include <vector>

namespace spm
{
    struct InstalledPackage
    {
        std::string name;
        std::string version;
        std::string installPath;
    };

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
        bool removePackage(const std::string &name);

        bool isPackageRegistered(const std::string &name);
        bool isPackageRegistered(const std::string &name, const std::string &version);

        std::string getPackageInstallPath(const std::string &name);

        std::vector<InstalledPackage> getInstalledPackages();

    private:
        sqlite3 *db_ = nullptr;
    };
}