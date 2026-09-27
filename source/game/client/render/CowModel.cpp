#include "game/client/render/CowModel.hpp"

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/client/render/ModelPartSkin.hpp"

CowModel::CowModel(const ModelDefinition& modelDefinition)
    : QuadrupedModel(12, 0.0f, modelDefinition)
{
    if (!modelDefinition.DoesOverrideModel())
    {
        mHead = new ModelPartSkin(this, 0, 0, NULL);
        mHead->addBox(-4.0f, -4.0f, -6.0f, 8, 8, 6, 0.0f);
        mHead->setPos(0.0f, 4.0f, -8.0f);

        mBody = new ModelPartSkin(this, 18, 4, NULL);
        mBody->addBox(-6.0f, -10.0f, -7.0f, 12, 18, 10, 0.0f);
        mBody->setPos(0.0f, 5.0f, 2.0f);
        mBody->texOffs(52, 0)->addBox(-2.0f, 2.0f, -8.0f, 4, 6, 1);

        mLeg0->mX -= 1.0f;
        mLeg1->mX += 1.0f;
        mLeg2->mX -= 1.0f;
        mLeg3->mX += 1.0f;
        mLeg2->mY -= 1.0f;
        mLeg3->mY -= 1.0f;

        mZHeadOffs += 2.0f;
        compileSkinnedDrawlist();
    }
    else
    {
        mZHeadOffs += 2.0f;
    }
}

CowModel::~CowModel()
{
}

std::wstring CowModel::getName()
{
    return L"cow";
}
