#include "game/core/AreaHint.hpp"

#include "game/core/Minecraft.hpp"
#include "game/util/AABB.hpp"
#include "game/world/generation/Player.hpp"

AreaHint::AreaHint(eTutorial_Hint hint, Tutorial* tutorial, eTutorial_State state,
                   eTutorial_State completionState, int descriptionId,
                   double x0, double y0, double z0, double x1, double y1,
                   double z1, bool repeatMessage, bool insideArea)
    : TutorialHint(hint, tutorial, descriptionId, (eHintType)5, repeatMessage, true, -1),
      area(AABB::newPermanent(x0, y0, z0, x1, y1, z1)),
      insideArea(insideArea),
      state(state),
      completionState(completionState)
{
}

AreaHint::~AreaHint()
{
    delete area;
}

int AreaHint::tick()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if ((state == (eTutorial_State)-2 || tutorial->currentState == state) && active)
    {
        if (area->contains(minecraft->mPlayer->getPos(1.0f)) == insideArea)
        {
            if (completionState == (eTutorial_State)-1)
            {
                active = false;
                return descriptionId;
            }
            if (tutorial->isStateCompleted(completionState))
            {
                active = false;
                return -1;
            }
            return descriptionId;
        }
    }
    return -1;
}
