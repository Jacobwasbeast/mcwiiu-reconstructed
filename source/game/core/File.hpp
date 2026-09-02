#ifndef GAME_CORE_FILE_HPP
#define GAME_CORE_FILE_HPP

#include <string>
#include <vector>

class File;

class FileFilter
{
public:
    virtual ~FileFilter();

    virtual bool accept(const File& file) = 0;
};

class File
{
public:
    File();
    File(const std::wstring& path);
    File(const std::wstring& parent, const std::wstring& child);
    File(const File& parent, const std::wstring& child);
    ~File();

    std::wstring getPath() const;
    std::wstring getName() const;
    std::wstring getBaseName() const;
    std::wstring ext() const;
    File parentDir();

    bool exists() const;
    bool isDirectory() const;
    bool isFile() const;
    bool mkdir() const;
    bool mkdirs() const;
    bool _delete();
    bool renameTo(File destination);
    long long lastModified();
    long long length();

    std::vector<File*>* listFiles() const;
    std::vector<File*>* listFiles(FileFilter* filter) const;

    static int hash_fnct(const File& file);
    static bool eq_test(const File& first, const File& second);

private:
    std::wstring mPath;
};

#endif
