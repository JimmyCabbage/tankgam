#include "util/File.h"

#include <string>
#include <string_view>
#include <fstream>
#include <filesystem>
#include <algorithm>

#include <fmt/format.h>

#include "util/Log.h"

FileManager::FileManager(Log& log)
    : log{ log }
{
}

FileManager::~FileManager()
{
    for (zip_t* zip : zips)
    {
        zip_close(zip);
    }
}

std::vector<char> FileManager::readFileRaw(std::string_view fileName)
{
    log.logf("Reading file: %s", fileName.data());
    
    std::ifstream file{ fileName.data(), std::ios::binary | std::ios::ate };
    if (!file.is_open())
    {
        //throw std::runtime_error{ fmt::format("Failed to read file {}", fileName) };
        //console.logf("Failed to read file %s from disk, time to search assets files", fileName.data());

        //iterate backwards cause we wanna take priority of modded files
        for (size_t i = zips.size(); i --> 0;)
        {
            zip_t* zip = zips[i];

            zip_stat_t st{};
            zip_stat_init(&st);
            zip_stat(zip, fileName.data(), 0, &st);

            std::vector<char> buffer;
            buffer.resize(st.size);

            //todo: error checking
            zip_file_t* zipFile = zip_fopen(zip, fileName.data(), 0);
            if (!zipFile)
            {
                continue;
            }
            
            //console.logf("Found %s in assets file #%zu", fileName.data(), i);

            zip_fread(zipFile, buffer.data(), st.size);
            zip_fclose(zipFile);

            return buffer;
        }

        throw std::runtime_error{ fmt::format("Failed to read file {}", fileName) };
    }

    //get length of file
    file.seekg(0, std::ios::end);
    const size_t length = file.tellg();

    std::vector<char> buffer;
    buffer.resize(length);

    file.seekg(0, std::ios::beg);
    file.read(buffer.data(), length);
    
    return buffer;
}

void FileManager::writeFileRaw(std::string_view fileName, std::span<char> buffer)
{
    log.logf("Writing file: %s", fileName.data());
    
    std::ofstream file{ fileName.data(), std::ios::binary | std::ios::ate };
    if (!file.is_open())
    {
        throw std::runtime_error{ fmt::format("Failed to open file {}", fileName) };
    }

    file.write(buffer.data(), buffer.size());
}

std::stringstream FileManager::readFile(std::string_view fileName)
{
    const std::vector<char> buf = readFileRaw(fileName);
    std::string str{ buf.begin(), buf.end() };
    std::stringstream sstr{ std::move(str) };
    
    return sstr;
}

std::vector<std::string> FileManager::fileNamesInDir(std::string_view dirName)
{
    std::vector<std::string> fileNames;
    
    //iterate backwards cause we wanna take priority of modded files
    for (size_t i = zips.size(); i --> 0;)
    {
        zip_t* zip = zips[i];
        
        const zip_int64_t numEntries = zip_get_num_entries(zip, 0);
        for (zip_int64_t j = 0; j < numEntries; j++)
        {
            std::string name = zip_get_name(zip, j, 0);
            
            const size_t lastSlash = name.find_last_of('/');
            if (lastSlash == std::string::npos)
            {
                continue;
            }
            
            //make sure this is the right directory
            if (dirName != name.substr(0, lastSlash + 1))
            {
                continue;
            }
            
            //make sure we're not just adding the directory directly
            if (dirName == name)
            {
                continue;
            }
            
            //make sure this isn't a duplicate of an already found file
            if (std::find(fileNames.begin(), fileNames.end(), name) != fileNames.end())
            {
                continue;
            }
            
            fileNames.push_back(std::move(name));
        }
    }
    
    namespace fs = std::filesystem;
    
    //get native filesystem files
    if (fs::is_directory(dirName.data()))
    {
        for (const auto& entry : fs::directory_iterator{ dirName })
        {
            if (!entry.is_directory() && !entry.is_socket())
            {
                std::string name = entry.path().string();
                
                //only add if this isn't a duplicate
                if (std::find(fileNames.begin(), fileNames.end(), name) == fileNames.end())
                {
                    fileNames.push_back(std::move(name));
                }
            }
        }
    }
    
    return fileNames;
}

void FileManager::loadAssetsFile(std::filesystem::path path)
{
    const std::string pathStr = path.string();
    log.logf("Reading assets file: %s", pathStr.data());

    int err = 0;
    zip_t* handle = zip_open(pathStr.data(), ZIP_RDONLY, &err);
    if (!handle)
    {
        zip_error_t error{};
        zip_error_init_with_code(&error, err);
        const std::string errorStr = zip_error_strerror(&error);
        zip_error_fini(&error);

        throw std::runtime_error{ fmt::format("Failed to read assets file {}, err: {}", pathStr, errorStr) };
    }

    zips.push_back(handle);
}
