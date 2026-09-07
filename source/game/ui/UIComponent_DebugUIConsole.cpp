#include "game/ui/UIComponent_DebugUIConsole.hpp"
#include "game/ui/fuiFile.hpp"

UIComponent_DebugUIConsole::UIComponent_DebugUIConsole(int sceneType, void*, UILayer* layer)
    : UIScene(sceneType, layer)
{
    initialiseMovie();
    m_updateLabels = false;
}

UIComponent_DebugUIConsole::~UIComponent_DebugUIConsole()
{
}

unsigned int UIComponent_DebugUIConsole::mapElementsAndNames()
{
    UIScene::mapElementsAndNames();
    fuiRenderNode* root = m_movie->getRootNode();

    m_consoleLines[0].setupControl(this, root, "consoleLine1");
    m_controls.push_back(&m_consoleLines[0]);
    m_consoleLines[1].setupControl(this, root, "consoleLine2");
    m_controls.push_back(&m_consoleLines[1]);
    m_consoleLines[2].setupControl(this, root, "consoleLine3");
    m_controls.push_back(&m_consoleLines[2]);
    m_consoleLines[3].setupControl(this, root, "consoleLine4");
    m_controls.push_back(&m_consoleLines[3]);
    m_consoleLines[4].setupControl(this, root, "consoleLine5");
    m_controls.push_back(&m_consoleLines[4]);
    m_consoleLines[5].setupControl(this, root, "consoleLine6");
    m_controls.push_back(&m_consoleLines[5]);
    m_consoleLines[6].setupControl(this, root, "consoleLine7");
    m_controls.push_back(&m_consoleLines[6]);
    m_consoleLines[7].setupControl(this, root, "consoleLine8");
    m_controls.push_back(&m_consoleLines[7]);
    m_consoleLines[8].setupControl(this, root, "consoleLine9");
    m_controls.push_back(&m_consoleLines[8]);
    m_consoleLines[9].setupControl(this, root, "consoleLine10");
    m_controls.push_back(&m_consoleLines[9]);
    return 1;
}

void UIComponent_DebugUIConsole::tick()
{
    UIScene::tick();
    if (!m_updateLabels)
        return;

    m_updateLabels = false;
    std::deque<std::string>::const_iterator text = m_text.begin();
    unsigned int index = 0;
    while (index < m_text.size() && index < 10)
    {
        m_consoleLines[index].setLabel(UIString(*text), false, false);
        ++text;
        ++index;
    }
}

std::wstring UIComponent_DebugUIConsole::getMoviePath()
{
    return std::wstring(L"DebugUIConsoleComponent");
}

int UIComponent_DebugUIConsole::getSceneType()
{
    return 68;
}

bool UIComponent_DebugUIConsole::stealsFocus()
{
    return false;
}

bool UIComponent_DebugUIConsole::hasFocus(int)
{
    return false;
}

bool UIComponent_DebugUIConsole::hidesLowerScenes()
{
    return false;
}
