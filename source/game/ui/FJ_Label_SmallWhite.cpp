#include "game/ui/FJ_Label_SmallWhite.hpp"

#include "game/ui/fuiRenderNode.hpp"

FJ_Label_SmallWhite::FJ_Label_SmallWhite(fuiRenderNode* renderNode)
    : FJ_Label(renderNode)
{
}

FJ_Label_SmallWhite::~FJ_Label_SmallWhite()
{
}

FJ_Label* FJ_Label_SmallWhite::Create(fuiRenderNode* renderNode)
{
    return new FJ_Label_SmallWhite(renderNode);
}

unsigned int FJ_Label_SmallWhite::getType()
{
    return 0x8084;
}
