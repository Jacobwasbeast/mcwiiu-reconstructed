#include "game/entity/ai/TurtleBreedGoal.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/ExperienceOrb.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/GameRules.hpp"
#include "game/core/Turtle.hpp"
#include "game/entity/mob/Animal.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Player.hpp"

TurtleBreedGoal::TurtleBreedGoal(Turtle* turtle, double speedModifier)
    : BreedGoal(turtle, speedModifier),
      mTurtle(turtle)
{
}

TurtleBreedGoal::~TurtleBreedGoal()
{
}

bool TurtleBreedGoal::canUse()
{
    return BreedGoal::canUse() && !mTurtle->hasEgg();
}

void TurtleBreedGoal::breed()
{
    boost::shared_ptr<Player> player = mAnimal->getLoveCause();
    if (!player)
    {
        player = mPartner->getLoveCause();
    }

    if (player)
    {
        player->awardStat(gStats->get_breedEntity(mTurtle->GetType()),
                          GenericStats::param_breedEntity(mTurtle->GetType()));
    }

    mTurtle->setHasEgg(true);
    mAnimal->resetLove();
    mPartner->resetLove();

    if (mLevel->getGameRules()->getBoolean(4))
    {
        boost::shared_ptr<ExperienceOrb> orb(new ExperienceOrb(
            mLevel, mAnimal->mX, mAnimal->mY, mAnimal->mZ,
            mAnimal->getRandom()->nextInt(7) + 1));
        mLevel->addEntity(orb);
    }
}

std::wstring TurtleBreedGoal::getName()
{
    return std::wstring(L"Turtle_Breed");
}
