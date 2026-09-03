#include "game/ui/FJ_SaveMessage.hpp"

#include "game/ui/FJ_FuiNode.hpp"

namespace {

FJ_FuiNode* findChild(FJ_FuiNode* node, const char* name) {
    if (node == 0) {
        return 0;
    }
    return node->getChildByName(std::string(name));
}

}

FJ_SaveMessage::FJ_SaveMessage(fuiRenderNode* renderNode)
    : FJ_Document(renderNode) {
}

FJ_SaveMessage::~FJ_SaveMessage() {
}

FJ_Document* FJ_SaveMessage::Create(fuiRenderNode* renderNode) {
    return new FJ_SaveMessage(renderNode);
}

void FJ_SaveMessage::AutoResize() {
    FJ_FuiNode* description = findChild(this, "Description");
    FJ_FuiNode* confirm = findChild(this, "Confirm");
    FJ_FuiNode* background = findChild(this, "BackgroundPanel");
    if (description == 0 || confirm == 0 || background == 0) {
        return;
    }

    float descriptionBottom = description->getY() + description->getHeight();
    float confirmHeight = confirm->getHeight();
    float confirmY = descriptionBottom - confirm->getY();
    float confirmOffset = confirmHeight * 0.5f + confirmY;
    confirm->setY(confirm->getY() + confirmOffset);

    FJ_FuiNode* graphic = confirm->getChildAt(0);
    if (graphic != 0) {
        graphic->setY(graphic->getY() + confirmOffset);
    }

    int count = getNumChildren();
    int i;
    for (i = 0; i < count; ++i) {
        FJ_FuiNode* child = getChildAt(i);
        if (child != 0) {
            child->setY(child->getY() - confirmOffset * 0.5f);
        }
    }

    background->setHeight(background->getHeight() + confirmOffset);
}

unsigned int FJ_SaveMessage::getType() {
    return 3;
}
