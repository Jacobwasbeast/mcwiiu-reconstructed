#include "game/core/BannerPattern.hpp"
#include "game/item/ItemInstance.hpp"

static std::vector<const BannerPattern*> bannerPatterns;

BannerPattern::BannerPattern(unsigned int id, const std::wstring& name, int* names, wchar_t tab)
    : patternItem(NULL)
{
    _init(id, name, names, tab);
}

BannerPattern::BannerPattern(unsigned int id, const std::wstring& name, const std::wstring& p0,
                             const std::wstring& p1, const std::wstring& p2, int* names, wchar_t tab)
    : patternItem(NULL)
{
    _init(id, name, names, tab);
    patterns[0] = p0;
    patterns[1] = p1;
    patterns[2] = p2;
}

BannerPattern::BannerPattern(unsigned int id, const std::wstring& name, ItemInstance* item,
                             int* names, wchar_t tab)
    : patternItem(item)
{
    _init(id, name, names, tab);
}

void BannerPattern::_init(unsigned int id, const std::wstring& name, int* names, wchar_t tab)
{
    atlasIndex = id;
    hashname = name;
    patternItem = NULL;
    displayName = names;
    craftingMenuTab = (unsigned short)tab;
    bannerPatterns.push_back(this);
}

const wchar_t* BannerPattern::getDisplayName(int index) const
{
    return (const wchar_t*)displayName[index];
}

BannerPattern* BannerPattern::byHash(const std::wstring& name)
{
    std::vector<const BannerPattern*>::const_iterator it;
    for (it = bannerPatterns.begin(); it != bannerPatterns.end(); ++it)
    {
        if ((*it)->hashname == name)
            return const_cast<BannerPattern*>(*it);
    }
    return NULL;
}

std::wstring BannerPattern::getHashname() const
{
    return hashname;
}

void BannerPattern::staticCtor()
{
    new BannerPattern(0, L"b", (int*)NULL, 0x30);
    new BannerPattern(1, L"bo", L"###", L"# #", L"###", (int*)NULL, 0x34);
    new BannerPattern(2, L"mc", (ItemInstance*)NULL, (int*)NULL, 0x34);
    new BannerPattern(3, L"cr", L"###", L"# #", L"###", (int*)NULL, 0x33);
    new BannerPattern(4, L"ld", (ItemInstance*)NULL, (int*)NULL, 0x35);
    new BannerPattern(5, L"rd", L"#  ", L" ##", L"  #", (int*)NULL, 0x31);
    new BannerPattern(6, L"hh", (ItemInstance*)NULL, (int*)NULL, 0x34);
    new BannerPattern(7, L"vh", L"# #", L"# #", L"# #", (int*)NULL, 0x32);
    new BannerPattern(8, L"mr", L"# #", L"###", L"# #", (int*)NULL, 0x32);
    new BannerPattern(9, L"ss", L"###", L"#  ", L"###", (int*)NULL, 0x32);
    new BannerPattern(10, L"bl", L"#  ", L"#  ", L"###", (int*)NULL, 0x32);
    new BannerPattern(11, L"br", L"  #", L"  #", L"###", (int*)NULL, 0x32);
    new BannerPattern(12, L"tl", L"###", L"#  ", L"#  ", (int*)NULL, 0x32);
    new BannerPattern(13, L"tr", L"###", L"  #", L"  #", (int*)NULL, 0x34);
    new BannerPattern(14, L"sc", L"###", L"# #", L"# #", (int*)NULL, 0x34);
    new BannerPattern(15, L"bs", L"###", L"  #", L"###", (int*)NULL, 0x32);
    new BannerPattern(16, L"cs", L"# #", L"###", L"# #", (int*)NULL, 0x32);
    new BannerPattern(17, L"ls", L"#  ", L"# #", L"###", (int*)NULL, 0x32);
    new BannerPattern(18, L"ms", L"###", L"###", L"###", (int*)NULL, 0x32);
    new BannerPattern(19, L"rs", L"  #", L"# #", L"###", (int*)NULL, 0x32);
    new BannerPattern(20, L"ts", L"###", L"  #", L"  #", (int*)NULL, 0x32);
    new BannerPattern(21, L"bt", L"# #", L"# #", L"###", (int*)NULL, 0x32);
    new BannerPattern(22, L"tt", L"# #", L"# #", L"###", (int*)NULL, 0x32);
    new BannerPattern(23, L"fl", L" # ", L"###", L" # ", (int*)NULL, 0x32);
    new BannerPattern(24, L"cre", L"###", L"# #", L"###", (int*)NULL, 0x32);
    new BannerPattern(25, L"sku", L"###", L"# #", L"###", (int*)NULL, 0x32);
    new BannerPattern(26, L"moj", L"###", L"###", L"###", (int*)NULL, 0x32);
    new BannerPattern(27, L"gra", L"###", L"# #", L"###", (int*)NULL, 0x32);
    new BannerPattern(28, L"bts", L"###", L"# #", L"###", (int*)NULL, 0x32);
    new BannerPattern(29, L"tts", L"###", L"# #", L"###", (int*)NULL, 0x32);
    new BannerPattern(30, L"hhb", L"###", L"# #", L"###", (int*)NULL, 0x32);
    new BannerPattern(31, L"vhr", L"# #", L"###", L"# #", (int*)NULL, 0x32);
    new BannerPattern(32, L"dls", L"#  ", L" ##", L"  #", (int*)NULL, 0x32);
    new BannerPattern(33, L"drs", L"  #", L"## ", L"#  ", (int*)NULL, 0x32);
    new BannerPattern(34, L"bts2", L"###", L"# #", L"###", (int*)NULL, 0x32);
    new BannerPattern(35, L"tts2", L"###", L"# #", L"###", (int*)NULL, 0x32);
    new BannerPattern(36, L"bo2", L"###", L"# #", L"###", (int*)NULL, 0x32);
    new BannerPattern(37, L"mc2", L"###", L"# #", L"###", (int*)NULL, 0x32);
}

unsigned long long BannerPattern::getPatterns() const
{
    return ((unsigned long long)3 << 32) | (unsigned long long)(unsigned long)(patterns);
}

unsigned int BannerPattern::hasPattern() const
{
    return (patternItem != NULL || !patterns[0].empty()) ? 1U : 0U;
}

bool BannerPattern::hasPatternItem() const
{
    return patternItem != NULL;
}

ItemInstance* BannerPattern::getPatternItem() const
{
    return patternItem;
}

unsigned int BannerPattern::getAtlasIndex() const
{
    return atlasIndex;
}

unsigned short BannerPattern::getCraftingMenuTab() const
{
    return craftingMenuTab;
}
