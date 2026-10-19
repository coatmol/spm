#include <iostream>
#include <CLI/CLI.hpp>
#include <nlohmann/json.hpp>

#include "downloader.hpp"
#include "database.hpp"
#include "extractor.hpp"
#include "utils.hpp"

int main(int argc, char **argv)
{
    CLI::App app{"spm - Simple Package Manager"};

    std::string rawinput;
    bool autoConfirm = false;

    auto *installCmd = app.add_subcommand("install", "Install a package");
    installCmd->add_option("package", rawinput, "Name of the package to install")->required();
    installCmd->add_flag("-y,--yes", autoConfirm, "Automatically confirm prompts");

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

        spm::Database db(spm::getDatabasePath());
        db.initializeSchema();

        if (db.isPackageRegistered(packageName, targetVersion))
        {
            std::cout << "[spm] Package '" << packageName << "' (v" << (targetVersion.empty() ? "latest" : targetVersion) << ") is already installed.\n";
            return 0;
        }
        else if (db.isPackageRegistered(packageName))
        {
            std::cout << "[spm] Package '" << packageName << "' is already installed, but not the requested version. Proceeding with installation of v" << targetVersion << ".\n";
        }

        if (!pkgEntry["versions"].contains(targetVersion))
        {
            std::cerr << "[spm] Version '" << targetVersion << "' for package '" << packageName << "' does not exist!\n";
            return 1;
        }

        std::string downloadUrl = pkgEntry["versions"][targetVersion]["url"];
        std::cout << "[spm] Found target version: " << targetVersion << "\n";
        std::cout << "[spm] Download URL: " << downloadUrl << "\n";

        char confirmInput = 'n';

        if (autoConfirm)
        {
            std::cout << "[spm] Skipping confirmation (-y enabled)...\n";
        }
        else
        {
            std::cout << "[spm] Proceed with installation? (y/N): ";
            std::cin >> confirmInput;
            confirmInput = std::tolower(confirmInput);

            if (confirmInput != 'y')
            {
                std::cout << "[spm] Installation aborted by user.\n";
                return 0;
            }
        }

        std::string baseDir = spm::getUserDirectory();
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
        std::cout << "[spm] Extracting package archive...\n";

        if (!spm::Extractor::extractZip(targetArchivePath, targetInstallDir))
        {
            std::cerr << "[spm] Failed to extract package archive.\n";
            return 1;
        }

        std::filesystem::remove(targetArchivePath);

        if (db.registerPackage(packageName, targetVersion, targetInstallDir))
        {
            std::cout << "[spm] Successfully installed " << packageName << " v" << targetVersion << " into " << targetInstallDir << "!\n";
        }

        spm::linkExecutables(targetInstallDir, baseDir + "/bin");
        spm::addToUserPath(baseDir + "/bin");

        std::cout << "[spm] Installation complete. You may need to restart your terminal for PATH changes to take effect.\n";
    }
    else
    {
        std::cout << "Usage: spm install <package-name>\n";
    }

    return 0;
}