#include "FileHelper.h"

#include <fstream>
#include <iostream>

std::vector<char> FileHelper::ReadFile(const std::string &fileName)
{
    std::ifstream file(fileName, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error("failed to open file!");
    }

    const size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}

void FileHelper::GetFilesFromDirectory(const std::string &folderPath, std::vector<fs::path> &files,
                                       const std::vector<std::string> &extensionsToIgnore,
                                       const std::string &extension, bool recursive)
{
    try
    {
        if (recursive)
        {
            for (const auto &entry : fs::recursive_directory_iterator(folderPath))
            {
                if (entry.is_regular_file() &&
                    (extension.empty() || entry.path().extension().string() == extension))
                {
                    if (extensionsToIgnore.size() != 0)
                    {
                        const std::string extensionStr = entry.path().extension().string();
                        auto it = std::ranges::find_if(extensionsToIgnore.begin(),
                                                       extensionsToIgnore.end(),
                                                       [extensionStr](const std::string &iterator)
                                                       { return iterator == extensionStr; });

                        if (it != extensionsToIgnore.end())
                        {
                            continue;
                        }
                    }

                    files.push_back(entry.path());
                }
            }
        }
        else
        {
            for (const auto &entry : fs::directory_iterator(folderPath))
            {
                if (entry.is_regular_file() &&
                    (extension.empty() || entry.path().extension().string() == extension))
                {
                    if (extensionsToIgnore.size() != 0)
                    {
                        const std::string extensionStr = entry.path().extension().string();
                        auto it = std::ranges::find_if(extensionsToIgnore.begin(),
                                                       extensionsToIgnore.end(),
                                                       [extensionStr](const std::string &iterator)
                                                       { return iterator == extensionStr; });

                        if (it != extensionsToIgnore.end())
                        {
                            continue;
                        }
                    }

                    files.push_back(entry.path());
                }
            }
        }
    }
    catch (const fs::filesystem_error &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}
