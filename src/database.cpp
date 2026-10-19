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

} // namespace spm