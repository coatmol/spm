#include <iostream>
#include <CLI/CLI.hpp>
#include <nlohmann/json.hpp>

int main(int argc, char** argv) {
    CLI::App app{"spm - A lightweight package manager"};

    std::string packageName;
    
    auto* installCmd = app.add_subcommand("install", "Install a package");
    installCmd->add_option("package", packageName, "Name of the package to install")->required();

    CLI11_PARSE(app, argc, argv);

    // Handle the commands
    if (*installCmd) {
        std::cout << "[spm] Installing package: " << packageName << "...\n";
        
        // Test that nlohmann/json is working properly
        nlohmann::json manifest = {
            {"name", packageName},
            {"status", "pending_download"}
        };
        std::cout << "[spm] Generated manifest config:\n" << manifest.dump(4) << "\n";
    } else {
        std::cout << "Usage: spm install <package-name>\n";
    }

    return 0;
}