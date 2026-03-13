#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

class FileHelper
{
  public:
    static std::vector<char> ReadFile(const std::string &fileName);
    static void GetFilesFromDirectory(const std::string &folderPath, std::vector<fs::path> &files,
                                      const std::vector<std::string> &extensionsToIgnore,
                                      const std::string &extension, bool recursive);
};