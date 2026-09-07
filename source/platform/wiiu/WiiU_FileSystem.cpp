#include "platform/wiiu/WiiU_FileSystem.hpp"

#include "platform/wiiu/CStorage.hpp"

#include <cstdio>
#include <string>

extern CStorage* gStorage;

void getFinalFilePath(char (&path)[256], const char* fileName)
{
    std::string mountedPath = gStorage->GetMountedPath(std::string(fileName));
    const std::string& filePath = mountedPath.empty() ? std::string(fileName) : mountedPath;
    const char* format = !filePath.empty() && filePath[0] == '/' ? "%s" : "/vol/content/%s";

    std::sprintf(path, format, filePath.c_str());
    for (char* character = path; *character != '\0'; ++character)
        if (*character == '\\')
            *character = '/';
}
