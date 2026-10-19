#pragma once

#include <windows.h>
#include <string>
#include <filesystem>
#include <iostream>

namespace spm
{

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

    bool addToUserPath(const std::string &binDir)
    {
        HKEY hKey;
        // Open the current user's environment variables registry key
        LONG result = RegOpenKeyExA(HKEY_CURRENT_USER, "Environment", 0, KEY_READ | KEY_WRITE, &hKey);
        if (result != ERROR_SUCCESS)
        {
            std::cerr << "[spm] Failed to open user environment registry key.\n";
            return false;
        }

        // Read the existing PATH value
        char pathBuffer[4096];
        DWORD bufferSize = sizeof(pathBuffer);
        DWORD regType = REG_SZ;
        result = RegQueryValueExA(hKey, "Path", NULL, &regType, (LPBYTE)pathBuffer, &bufferSize);

        std::string currentPath = (result == ERROR_SUCCESS) ? std::string(pathBuffer) : "";

        // Check if our bin directory is already in the PATH
        if (currentPath.find(binDir) != std::string::npos)
        {
            RegCloseKey(hKey);
            return true;
        }

        // Append our bin directory to the existing PATH
        std::string newPath = currentPath;
        if (!newPath.empty() && newPath.back() != ';')
        {
            newPath += ";";
        }
        newPath += binDir;

        // Write the updated PATH back to the registry
        result = RegSetValueExA(hKey, "Path", 0, REG_EXPAND_SZ, (const BYTE *)newPath.c_str(), static_cast<DWORD>(newPath.length() + 1));
        RegCloseKey(hKey);

        if (result != ERROR_SUCCESS)
        {
            std::cerr << "[spm] Failed to write updated PATH to registry.\n";
            return false;
        }

        // Broadcast a system message so open shells know the environment changed
        SendMessageTimeoutA(HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM) "Environment", SMTO_ABORTIFHUNG, 5000, NULL);

        std::cout << "[spm] Added " << binDir << " to user PATH successfully!\n";
        return true;
    }

    bool linkExecutables(const std::string &appInstallDir, const std::string &binDir)
    {
        namespace fs = std::filesystem;
        fs::create_directories(binDir);

        int linkedCount = 0;

        for (auto const &dir_entry : fs::recursive_directory_iterator(appInstallDir))
        {
            if (dir_entry.is_regular_file() && dir_entry.path().extension() == ".exe")
            {
                fs::path exePath = dir_entry.path();
                std::string stemName = exePath.filename().stem().string();
                std::string fileName = exePath.filename().string();

                fs::path symlinkPath = fs::path(binDir) / fileName;
                fs::path batPath = fs::path(binDir) / (stemName + ".bat");

                // Clean up any existing conflicting links or wrappers
                if (fs::exists(symlinkPath))
                    fs::remove(symlinkPath);
                if (fs::exists(batPath))
                    fs::remove(batPath);

                // Try creating a true symbolic link first
                std::error_code ec;
                fs::create_symlink(exePath, symlinkPath, ec);

                if (!ec)
                {
                    std::cout << "[spm] Linked via symlink: " << fileName << " -> PATH\n";
                    linkedCount++;
                }
                else
                {
                    // Fallback to .bat wrapper if symlinks are restricted
                    std::ofstream batFile(batPath);
                    if (batFile.is_open())
                    {
                        batFile << "@echo off\r\n";
                        batFile << "\"" << fs::absolute(exePath).string() << "\" %*\r\n";
                        batFile.close();

                        std::cout << "[spm] Linked via wrapper: " << stemName << " -> PATH\n";
                        linkedCount++;
                    }
                    else
                    {
                        std::cerr << "[spm] Warning: Failed to link " << fileName << "\n";
                    }
                }
            }
        }

        if (linkedCount > 0)
        {
            std::cout << "[spm] Successfully linked " << linkedCount << " executable(s) to PATH!\n";
        }

        return linkedCount > 0;
    }
}
