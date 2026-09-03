#ifndef GAME_CORE_BANNERPATTERN_HPP
#define GAME_CORE_BANNERPATTERN_HPP

#include <string>
#include <vector>

class ItemInstance;

class BannerPattern
{
public:
    BannerPattern(unsigned int id, const std::wstring& hashname, int* displayName, wchar_t craftingMenuTab);
    BannerPattern(unsigned int id, const std::wstring& hashname, const std::wstring& pattern0,
                  const std::wstring& pattern1, const std::wstring& pattern2, int* displayName,
                  wchar_t craftingMenuTab);
    BannerPattern(unsigned int id, const std::wstring& hashname, ItemInstance* patternItem,
                  int* displayName, wchar_t craftingMenuTab);

    const wchar_t* getDisplayName(int index) const;
    static BannerPattern* byHash(const std::wstring& hashname);
    std::wstring getHashname() const;
    void _init(unsigned int id, const std::wstring& hashname, int* displayName, wchar_t craftingMenuTab);
    static void staticCtor();
    unsigned long long getPatterns() const;
    unsigned int hasPattern() const;
    bool hasPatternItem() const;
    ItemInstance* getPatternItem() const;
    unsigned int getAtlasIndex() const;
    unsigned short getCraftingMenuTab() const;

private:
    std::wstring hashname;
    std::wstring patterns[3];
    ItemInstance* patternItem;
    unsigned int atlasIndex;
    unsigned short craftingMenuTab;
    unsigned short reserved;
    int* displayName;
};

#endif
