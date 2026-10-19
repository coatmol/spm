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

std::string getUserDirectory()
{
    std::filesystem::path appsDir;
    const char *localAppData = std::getenv("LOCALAPPDATA");
    if (localAppData)
    {
        appsDir = localAppData;
    }
    else
    {
        appsDir = std::filesystem::current_path(); // Fallback
    }
    appsDir /= "spm";
    return appsDir.string();
}

int main(int argc, char **argv)
{
    CLI::App app{"spm - Simple Package Manager"};

    std::string rawinput;

    auto *installCmd = app.add_subcommand("install", "Install a package");
    installCmd->add_option("package", rawinput, "Name of the package to install")->required();

    CLI11_PARSE(app, argc, argv);

    if (*installCmd)
    {
        std::string packageName = rawinput;
        std::string requestedVersion = "";

        size_t atPos = packageName.find('@');
        if (atPos != std::string::npos)
        {
            requestedVersion = packageName.substr(atPos + 1);
            packageName = packageName.substr(0, atPos);
        }

        std::cout << "[spm] Looking up package: " << packageName << " (v" << (requestedVersion == "" ? "latest" : requestedVersion) << ")...\n";

        std::string registryHost = "raw.githubusercontent.com";
        std::string registryPath = "/coatmol/spm-registry/master/registry.json"; // TODO: Let users configure this via a config file

        std::string registryJsonStr;
        if (!spm::Downloader::downloadText(registryHost, registryPath, registryJsonStr))
        {
            std::cerr << "[spm] Failed to fetch registry index from GitHub.\n";
            return 1;
        }

        auto registry = nlohmann::json::parse(registryJsonStr);
        if (!registry["packages"].contains(packageName))
        {
            std::cerr << "[spm] Package '" << packageName << "' not found in registry!\n";
            return 1;
        }

        auto pkgEntry = registry["packages"][packageName];
        std::string targetVersion = requestedVersion.empty() ? pkgEntry["latest"] : requestedVersion;

        if (!pkgEntry["versions"].contains(targetVersion))
        {
            std::cerr << "[spm] Version '" << targetVersion << "' for package '" << packageName << "' does not exist!\n";
            return 1;
        }

        std::string downloadUrl = pkgEntry["versions"][targetVersion]["url"];
        std::cout << "[spm] Found target version: " << targetVersion << "\n";
        std::cout << "[spm] Download URL: " << downloadUrl << "\n";

        // Set up local paths and download archive
        std::string baseDir = getUserDirectory();
        std::string tempDir = baseDir + "/temp";
        std::string targetInstallDir = baseDir + "/apps/" + packageName;
        std::string targetArchivePath = tempDir + "/" + packageName + ".zip";

        std::filesystem::create_directories(tempDir);
        std::filesystem::create_directories(targetInstallDir);

        std::cout << "[spm] Downloading package archive...\n";

        size_t hostEndPos = downloadUrl.find('/', downloadUrl.find("://") + 3);
        if (!spm::Downloader::downloadFile(downloadUrl.substr(0, hostEndPos), downloadUrl.substr(hostEndPos), targetArchivePath))
        {
            std::cerr << "[spm] Failed to download package archive from: " << downloadUrl << "\n";
            return 1;
        }

        // Extract and Register in SQLite
        spm::Database db(getDatabasePath());
        db.initializeSchema();

        if (db.registerPackage(packageName, targetVersion, targetInstallDir))
        {
            std::cout << "[spm] Successfully installed " << packageName << " v" << targetVersion << " into " << targetInstallDir << "!\n";
        }

        std::filesystem::remove(targetArchivePath);
    }
    else
    {
        std::cout << "Usage: spm install <package-name>\n";
    }

    return 0;
}