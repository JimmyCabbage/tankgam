#pragma once

#include <filesystem>
#include <sstream>
#include <vector>
#include <span>
#include <utility>
#include <cstdint>

#include <zip.h>

class Engine;
class Console;

class FileManager
{
public:
    explicit FileManager(Engine& engine);
    ~FileManager();
    
    std::vector<char> readFileRaw(std::string_view fileName);
    
    void writeFileRaw(std::string_view fileName, std::span<char> buffer);
    
    std::stringstream readFile(std::string_view fileName);
    
    void loadAssetsFile(std::filesystem::path path);

private:
    Console& console;

    std::vector<zip_t*> zips;
};
