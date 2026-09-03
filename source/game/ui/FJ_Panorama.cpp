#include "game/ui/FJ_Panorama.hpp"
#include "game/ui/FJ_FuiNode.hpp"

FJ_Panorama::FJ_Panorama(fuiRenderNode* renderNode) : FJ_Document(renderNode)
{
    ShowPanoramaDay(true);
}

FJ_Panorama::~FJ_Panorama()
{
}

FJ_Panorama* FJ_Panorama::Create(fuiRenderNode* renderNode)
{
    return new FJ_Panorama(renderNode);
}

void FJ_Panorama::ShowPanoramaDay(bool day)
{
    FJ_FuiNode* dayNode = getChildByName(std::string("Panorama_Day"));
    if (dayNode != NULL)
    {
        FJ_FuiNode* dayFrame = dayNode->getChildAt(0);
        if (dayFrame != NULL) dayFrame->setVisible(day);
    }
    FJ_FuiNode* nightNode = getChildByName(std::string("Panorama_Night"));
    if (nightNode != NULL)
    {
        FJ_FuiNode* nightFrame = nightNode->getChildAt(0);
        if (nightFrame != NULL) nightFrame->setVisible(!day);
    }
}

int FJ_Panorama::GetCurrentFrame()
{
    FJ_FuiNode* dayNode = getChildByName(std::string("Panorama_Day"));
    if (dayNode == NULL) return 0;
    FJ_FuiNode* frame = dayNode->getChildAt(0);
    return frame == NULL ? 0 : static_cast<int>(frame->currentFrame());
}

void FJ_Panorama::SetCurrentFrame(int frame)
{
    FJ_FuiNode* dayNode = getChildByName(std::string("Panorama_Day"));
    FJ_FuiNode* nightNode = getChildByName(std::string("Panorama_Night"));
    FJ_FuiNode* target = dayNode == NULL ? NULL : dayNode->getChildAt(0);
    if (target == NULL && nightNode != NULL) target = nightNode->getChildAt(0);
    if (target != NULL) target->gotoAndPlay(frame);
}

unsigned int FJ_Panorama::getType()
{
    return 4;
}
