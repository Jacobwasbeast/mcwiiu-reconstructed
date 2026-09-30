#include "game/ui/FJ_SaveMessage.hpp"

#include "game/ui/FJ_FuiNode.hpp"
#include "game/util/Math.hpp"

FJ_SaveMessage::FJ_SaveMessage(fuiRenderNode* renderNode)
    : FJ_Document(renderNode) {
}

FJ_Document* FJ_SaveMessage::Create(fuiRenderNode* renderNode) {
    return new FJ_SaveMessage(renderNode);
}

void FJ_SaveMessage::AutoResize() {
    FJ_FuiNode* description = getChildByName(std::string("Description"))->asMovieClip();
    FJ_FuiNode* confirm = getChildByName(std::string("Confirm"))->asMovieClip();
    FJ_FuiNode* background = getChildByName(std::string("BackgroundPanel"))->asMovieClip();

    float descriptionBottom = description->getY() + description->getHeight();
    float confirmOffset = static_cast<float>(
        Math::round(confirm->getHeight() * 0.5f + (descriptionBottom - confirm->getY())));
    confirm->setY(confirm->getY() + confirmOffset);

    FJ_FuiNode* graphic = background->getChildAt(0)->asMovieClip();
    graphic->setHeight(graphic->getHeight() + confirmOffset);

    int count = getNumChildren();
    float childOffset = static_cast<float>(Math::round(confirmOffset * 0.5f));
    int i;
    for (i = 0; i < count; ++i) {
        FJ_FuiNode* child = getChildAt(i);
        child->setY(child->getY() - childOffset);
    }
}

unsigned int FJ_SaveMessage::getType() {
    return 3;
}
