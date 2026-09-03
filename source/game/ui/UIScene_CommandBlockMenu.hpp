#ifndef GAME_UI_UISCENE_COMMANDBLOCKMENU_HPP
#define GAME_UI_UISCENE_COMMANDBLOCKMENU_HPP

#include <string>

class UILayer;
class CommandBlockEntity;

class UIScene_CommandBlockMenu {
public:
    UIScene_CommandBlockMenu(int type, void* data, UILayer* layer);
    ~UIScene_CommandBlockMenu();
    std::wstring getMoviePath();
    void Initialise(CommandBlockEntity* commandBlock);
    void ConfirmButtonClicked();

private:
    void* m_data;
    int m_type;
    CommandBlockEntity* m_commandBlock;
    UILayer* m_layer;
};

#endif
