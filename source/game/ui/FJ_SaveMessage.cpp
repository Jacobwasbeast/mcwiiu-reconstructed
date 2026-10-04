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
    float confirmY = confirm->getY();
    float confirmHeight = confirm->getHeight();
    float confirmOffset = static_cast<float>(
        Math::round(confirmHeight * 0.5f + (descriptionBottom - confirmY)));
    confirm->setY(confirm->getY() + confirmOffset);

    FJ_FuiNode* graphic = background->getChildAt(0)->asMovieClip();
    float graphicHeight = graphic->getHeight();
    graphic->setHeight(graphicHeight + confirmOffset);

    int i;
    for (i = 0; i < m_root->getNumChildren(); ++i) {
        FJ_FuiNode* child = getChildAt(i);
        child->setY(child->getY() - static_cast<float>(Math::round(confirmOffset * 0.5f)));
    }
}

unsigned int FJ_SaveMessage::getType() {
    return 3;
}
