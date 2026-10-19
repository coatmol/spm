#include "database.hpp"
#include <iostream>

namespace spm
{

    Database::Database(const std::string &dbPath)
    {
        if (sqlite3_open(dbPath.c_str(), &db_) != SQLITE_OK)
        {
            std::cerr << "[spm] Error opening database: " << sqlite3_errmsg(db_) << "\n";
            db_ = nullptr;
        }
    }

    Database::~Database()
    {
        if (db_)
        {
            sqlite3_close(db_);
        }
    }

    bool Database::initializeSchema()
    {
        if (!db_)
            return false;

        const char *sql = R"(
        CREATE TABLE IF NOT EXISTS installed_packages (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL UNIQUE,
            version TEXT NOT NULL,
            install_path TEXT NOT NULL,
            installed_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        );
    )";

        char *errMsg = nullptr;
        int rc = sqlite3_exec(db_, sql, nullptr, nullptr, &errMsg);
        if (rc != SQLITE_OK)
        {
            std::cerr << "[spm] SQL error initializing schema: " << errMsg << "\n";
            sqlite3_free(errMsg);
            return false;
        }
        return true;
    }

    bool Database::registerPackage(const std::string &name, const std::string &version, const std::string &installPath)
    {
        if (!db_)
            return false;

        std::string sql = "INSERT OR REPLACE INTO installed_packages (name, version, install_path) VALUES (?, ?, ?);";
        sqlite3_stmt *stmt = nullptr;

        if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        {
            std::cerr << "[spm] Failed to prepare statement: " << sqlite3_errmsg(db_) << "\n";
            return false;
        }

        sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, version.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, installPath.c_str(), -1, SQLITE_TRANSIENT);

        int rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);

        if (rc != SQLITE_DONE)
        {
            std::cerr << "[spm] Failed to execute insert: " << sqlite3_errmsg(db_) << "\n";
            return false;
        }

        return true;
    }

    bool Database::isPackageRegistered(const std::string &name)
    {
        if (!db_)
            return false;

        std::string sql = "SELECT COUNT(*) FROM installed_packages WHERE name = ?;";

        sqlite3_stmt *stmt = nullptr;

        if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        {
            std::cerr << "[spm] Failed to prepare statement: " << sqlite3_errmsg(db_) << "\n";
            return false;
        }

        sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);

        int rc = sqlite3_step(stmt);
        bool isRegistered = false;

        if (rc == SQLITE_ROW)
        {
            int count = sqlite3_column_int(stmt, 0);
            isRegistered = (count > 0);
        }
        else
        {
            std::cerr << "[spm] Failed to execute query: " << sqlite3_errmsg(db_) << "\n";
        }

        sqlite3_finalize(stmt);
        return isRegistered;
    }

    bool Database::isPackageRegistered(const std::string &name, const std::string &version)
    {
        if (!db_)
            return false;

        std::string sql = "SELECT COUNT(*) FROM installed_packages WHERE name = ? AND version = ?;";
        sqlite3_stmt *stmt = nullptr;

        if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        {
            std::cerr << "[spm] Failed to prepare statement: " << sqlite3_errmsg(db_) << "\n";
            return false;
        }

        sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, version.c_str(), -1, SQLITE_TRANSIENT);

        int rc = sqlite3_step(stmt);
        bool isRegistered = false;

        if (rc == SQLITE_ROW)
        {
            int count = sqlite3_column_int(stmt, 0);
            isRegistered = (count > 0);
        }
        else
        {
            std::cerr << "[spm] Failed to execute query: " << sqlite3_errmsg(db_) << "\n";
        }

        sqlite3_finalize(stmt);
        return isRegistered;
    }

    std::vector<InstalledPackage> Database::getInstalledPackages()
    {
        if (!db_)
            return std::vector<InstalledPackage>();

        std::vector<InstalledPackage> packages;
        sqlite3_stmt *stmt;
        const char *sql = "SELECT name, version, install_path FROM installed_packages;";

        if (sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr) == SQLITE_OK)
        {
            while (sqlite3_step(stmt) == SQLITE_ROW)
            {
                InstalledPackage pkg;
                pkg.name = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
                pkg.version = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
                pkg.installPath = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
                packages.push_back(pkg);
            }
            sqlite3_finalize(stmt);
        }
        return packages;
    }

} // namespace spm