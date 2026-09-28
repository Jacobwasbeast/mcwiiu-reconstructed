#include "game/core/ScrolledSelectionList.hpp"

#include "game/core/Button.hpp"

ScrolledSelectionList::ScrolledSelectionList(Minecraft* minecraft, int width, int height, int top, int bottom, int slotHeight)
    : minecraft(minecraft), width(width), height(height), top(top), bottom(bottom), right(width), left(0),
      slotHeight(slotHeight), scrollUpButtonId(0), scrollDownButtonId(0), initialClickY(0.0f),
      scrollMultiplier(0.0f), amountScrolled(0.0f), selectedElement(0), lastClicked(0),
      renderSelection(false), renderHeaderEnabled(false), headerPadding(0)
{
}

int ScrolledSelectionList::getMaxPosition()
{
    return getNumberOfItems() * slotHeight + headerPadding;
}

int ScrolledSelectionList::getItemAtPosition(int x, int y)
{
    int center = width / 2;
    int relativeY = (y - top) - headerPadding + (int)amountScrolled - 4;
    int item = relativeY / slotHeight;
    if (x >= center - 110 && x <= center + 110 && item >= 0 && relativeY >= 0 && item < getNumberOfItems())
    {
        return item;
    }
    return -1;
}

void ScrolledSelectionList::init(std::vector<Button*>* buttons, int scrollUpButtonId, int scrollDownButtonId)
{
    this->scrollUpButtonId = scrollUpButtonId;
    this->scrollDownButtonId = scrollDownButtonId;
}

void ScrolledSelectionList::capYPosition()
{
    int maxScroll = getMaxPosition() - ((height - top) - 4);
    maxScroll /= 2;
    if (amountScrolled < 0.0f)
    {
        amountScrolled = 0.0f;
    }
    if ((float)maxScroll < amountScrolled)
    {
        amountScrolled = (float)maxScroll;
    }
}

void ScrolledSelectionList::buttonClicked(Button* button)
{
    if (button->mActive)
    {
        if (button->mId == scrollUpButtonId)
        {
            amountScrolled -= (float)((slotHeight * 2) / 3);
            initialClickY = -2.0f;
            capYPosition();
        }
        else if (button->mId == scrollDownButtonId)
        {
            amountScrolled += (float)((slotHeight * 2) / 3);
            initialClickY = -2.0f;
            capYPosition();
        }
    }
}
