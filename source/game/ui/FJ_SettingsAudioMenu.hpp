#ifndef GAME_UI_FJ_SETTINGSAUDIOMENU_HPP
#define GAME_UI_FJ_SETTINGSAUDIOMENU_HPP

#include "game/ui/FJ_Document.hpp"

class fuiRenderNode;

class FJ_SettingsAudioMenu : public FJ_Document
{
public:
    FJ_SettingsAudioMenu(fuiRenderNode* renderNode);
    virtual ~FJ_SettingsAudioMenu();
    static FJ_SettingsAudioMenu* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
};

#endif
