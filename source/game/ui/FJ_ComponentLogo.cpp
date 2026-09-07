#include "game/ui/FJ_ComponentLogo.hpp"

#include <string>

namespace
{
    std::string gLogoLanguageLabels[3] = {"English", "Japanese", "Chinese"};
    std::string gComponentLogoStrings0[7];
    std::string gComponentLogoStrings1[5];

    void destroyStrings(std::string* strings, int count)
    {
        for (int i = 0; i < count; ++i)
            strings[i].~basic_string();
    }
}

void __T0()
{
    destroyStrings(gLogoLanguageLabels, 3);
}

void __T1()
{
    destroyStrings(gComponentLogoStrings0, 7);
}

void __T2()
{
    destroyStrings(gComponentLogoStrings1, 5);
}

FJ_ComponentLogo::FJ_ComponentLogo(fuiRenderNode* renderNode)
    : FJ_Document(renderNode), m_logoLanguage(-1)
{
    SetLogoLanguage(0);
}

FJ_ComponentLogo::~FJ_ComponentLogo()
{
}

void FJ_ComponentLogo::SetLogoLanguage(int language)
{
    if (m_logoLanguage != language)
    {
        FJ_FuiNode* logoObject = getChildByName(std::string("LogoObject"));
        FJ_FuiNode* logo = logoObject->getChildAt(0);
        logo->gotoAndStop(gLogoLanguageLabels[language]);
        m_logoLanguage = language;
    }
}

FJ_ComponentLogo* FJ_ComponentLogo::Create(fuiRenderNode* renderNode)
{
    FJ_ComponentLogo* logo = new FJ_ComponentLogo(renderNode);
    return logo;
}

unsigned int FJ_ComponentLogo::getType()
{
    return 5;
}
