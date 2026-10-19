#include <iostream>
#include <CLI/CLI.hpp>
#include <nlohmann/json.hpp>

#include "downloader.hpp"

int main(int argc, char** argv) {
    CLI::App app{"spm - Simple Package Manager"};

    std::string packageName;
    
    auto* installCmd = app.add_subcommand("install", "Install a package");
    installCmd->add_option("package", packageName, "Name of the package to install")->required();

    CLI11_PARSE(app, argc, argv);

    if (*installCmd) {
        std::cout << "[spm] Looking up package: " << packageName << "...\n";

        // Test downloading a JSON response from a public test API (e.g., httpbin.org)
        std::string responseBody;
        if (spm::Downloader::downloadText("https://httpbin.org", "/json", responseBody)) {
            std::cout << "[spm] Received registry metadata:\n";
            
            // Parse and pretty-print the JSON response
            auto jsonDoc = nlohmann::json::parse(responseBody);
            std::cout << jsonDoc.dump(2) << "\n";
        } else {
            std::cerr << "[spm] Failed to fetch package metadata.\n";
        }
    } else {
        std::cout << "Usage: spm install <package-name>\n";
    }

    return 0;
}