#include "game/ui/FJ_ComponentLogo.hpp"

#include <string>

namespace
{
    const char* gLogoLanguageLabels[3] = {"English", "Japanese", "Chinese"};
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
        logo->gotoAndStop(std::string(gLogoLanguageLabels[language]));
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
