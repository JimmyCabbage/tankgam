#pragma once

#include <filesystem>
#include <sstream>
#include <vector>
#include <span>
#include <utility>
#include <cstdint>
#include <string>

#include <zip.h>

class Log;

class FileManager
{
public:
    explicit FileManager(Log& log);
    ~FileManager();
    
    std::vector<char> readFileRaw(std::string_view fileName);
    
    void writeFileRaw(std::string_view fileName, std::span<char> buffer);
    
    std::stringstream readFile(std::string_view fileName);
    
    std::vector<std::string> fileNamesInDir(std::string_view dirName);
    
    void loadAssetsFile(std::filesystem::path path);

private:
    Log& log;

    std::vector<zip_t*> zips;
};
