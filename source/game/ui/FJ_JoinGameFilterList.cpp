#include "game/ui/FJ_JoinGameFilterList.hpp"

#include "game/ui/FJ_Event.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"

class FJ_Base
{
public:
    int getId();
};

class FJ_Button
{
public:
    FJ_Base* getObjListParent();
    int getId();
    void setButtonPressed(bool pressed);
};

FJ_JoinGameFilterList::FJ_JoinGameFilterList(fuiRenderNode* renderNode)
    : FJ_Document(renderNode)
{
}

FJ_Document* FJ_JoinGameFilterList::Create(fuiRenderNode* renderNode)
{
    return new FJ_JoinGameFilterList(renderNode);
}

unsigned int FJ_JoinGameFilterList::getType()
{
    return 0x3f;
}

void FJ_JoinGameFilterList::handlePress(FJ_Event* event)
{
    FJ_Button* button = event->m_pTarget->asFJ_Button();
    if (button != NULL)
    {
        FJ_Base* parent = button->getObjListParent();
        m_pCallbackScene->handlePress(parent->getId(), button->getId());
        button->setButtonPressed(false);
    }
}

FJ_JoinGameFilterList::~FJ_JoinGameFilterList()
{
}
