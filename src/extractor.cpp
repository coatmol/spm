#include "extractor.hpp"

#include <windows.h>
#include <filesystem>
#include <iostream>

namespace spm
{

    bool Extractor::extractZip(const std::string &archivePath, const std::string &destinationDir)
    {
        namespace fs = std::filesystem;

        if (!fs::exists(archivePath))
        {
            std::cerr << "[spm] Archive not found: " << archivePath << "\n";
            return false;
        }

        fs::create_directories(destinationDir);

        // Build the command line for Windows native tar
        std::string cmd = "tar -xf \"" + archivePath + "\" -C \"" + destinationDir + "\"";

        // Convert std::string to mutable LPSTR for CreateProcessA
        std::vector<char> cmdLine(cmd.begin(), cmd.end());
        cmdLine.push_back('\0');

        STARTUPINFOA si = {sizeof(si)};
        PROCESS_INFORMATION pi = {0};
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE; // Hides the annoying black terminal popup window

        // Execute the process
        if (!CreateProcessA(NULL, cmdLine.data(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
        {
            std::cerr << "[spm] Failed to start extraction process.\n";
            return false;
        }

        // Wait for extraction to finish
        WaitForSingleObject(pi.hProcess, INFINITE);

        DWORD exitCode = 0;
        GetExitCodeProcess(pi.hProcess, &exitCode);

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        if (exitCode != 0)
        {
            std::cerr << "[spm] Extraction failed with exit code: " << exitCode << "\n";
            return false;
        }

        return true;
    }

}