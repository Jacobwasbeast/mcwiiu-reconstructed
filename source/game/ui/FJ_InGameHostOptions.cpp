#include "game/ui/FJ_InGameHostOptions.hpp"

#include <string>

#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/FJ_List.hpp"
#include "game/ui/FJ_List_MultiList.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/util/Math.hpp"

FJ_InGameHostOptions::FJ_InGameHostOptions(fuiRenderNode* renderNode)
    : FJ_Document(renderNode)
{
}

FJ_Document* FJ_InGameHostOptions::Create(fuiRenderNode* renderNode)
{
    return new FJ_InGameHostOptions(renderNode);
}

unsigned int FJ_InGameHostOptions::getType()
{
    return 0x2d;
}

void FJ_InGameHostOptions::InitMenu()
{
    FJ_FuiNode* background = getChildByName(std::string("BackgroundPanel"));
    FJ_FuiNode* list = getChildByName(std::string("HostOptionsList"));
    FJ_FuiNode* backgroundItem = background->getChildAt(0);
    const float backgroundHeight = background->getHeight();
    FJ_List* hostOptionsList = static_cast<FJ_List*>(list->asFJ_List_MultiList());
    const int listHeight = hostOptionsList->GetListHeight();

    if (static_cast<float>(listHeight) < list->getHeight())
    {
        backgroundItem->setY(static_cast<float>(listHeight) +
                             (list->getY() - background->getY()) * 2.0f);

        const float amount = static_cast<float>(Math::round((backgroundHeight -
                                                              backgroundItem->getHeight()) * 0.5f));
        if (amount > 0.0f)
        {
            backgroundItem->incY(amount);
            list->incY(amount);
        }
    }
    SetFocus(-1);
}

void FJ_InGameHostOptions::SetSafeZone(int top, int bottom, int left, int right)
{
    FJ_Document::SetSafeZone(top, bottom, left, right);

    const double stageHeight = getStage()->getStageHeight();
    if (stageHeight != 360.0 && stageHeight != 540.0)
        return;

    setX(-static_cast<float>(top) - 176.0f);
}

FJ_InGameHostOptions::~FJ_InGameHostOptions()
{
}
