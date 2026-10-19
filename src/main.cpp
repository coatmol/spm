#include <iostream>
#include <CLI/CLI.hpp>
#include <nlohmann/json.hpp>

#include "downloader.hpp"
#include "database.hpp"

std::string getDatabasePath()
{
    std::filesystem::path spmDir;

    const char *userProfile = std::getenv("USERPROFILE");
    if (userProfile)
    {
        spmDir = userProfile;
    }
    else
    {
        spmDir = "."; // Fallback to current directory if USERPROFILE isn't found
    }

    // Append .spm directory and create it if it doesn't exist
    spmDir /= ".spm";
    std::filesystem::create_directories(spmDir);

    // Append the database filename
    spmDir /= "index.db";
    return spmDir.string();
}

int main(int argc, char **argv)
{
    CLI::App app{"spm - Simple Package Manager"};

    std::string packageName;

    auto *installCmd = app.add_subcommand("install", "Install a package");
    installCmd->add_option("package", packageName, "Name of the package to install")->required();

    CLI11_PARSE(app, argc, argv);

    if (*installCmd)
    {
        std::cout << "[spm] Looking up package: " << packageName << "...\n";

        std::string dbPath = getDatabasePath();
        std::cout << "[spm] Using state database at: " << dbPath << "\n";

        // Initialize the local SQLite database
        spm::Database db(dbPath);
        if (!db.initializeSchema())
        {
            std::cerr << "[spm] Failed to initialize local database state.\n";
            return 1;
        }

        // Fetch mock package metadata from the network
        std::string responseBody;
        if (spm::Downloader::downloadText("https://httpbin.org", "/json", responseBody))
        {
            std::cout << "[spm] Received registry metadata:\n";

            auto jsonDoc = nlohmann::json::parse(responseBody);
            std::cout << jsonDoc.dump(2) << "\n";

            // 3. Register the package installation locally in SQLite
            std::string dummyInstallPath = "C:/Program Files/spm/" + packageName;
            if (db.registerPackage(packageName, "1.0.0", dummyInstallPath))
            {
                std::cout << "[spm] Successfully recorded " << packageName << " in state database!\n";
            }
        }
        else
        {
            std::cerr << "[spm] Failed to fetch package metadata.\n";
        }
    }
    else
    {
        std::cout << "Usage: spm install <package-name>\n";
    }

    return 0;
}