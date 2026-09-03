#include "game/client/render/SquidModel.hpp"

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPartSkin.hpp"

#include <cmath>

SquidModel::SquidModel(const ModelDefinition& definition)
    : Model(), mBody(0), mTentacle1(0), mTentacle2(0), mTentacle3(0), mTentacle4(0),
      mTentacle5(0), mTentacle6(0), mTentacle7(0), mTentacle8(0)
{
    if (!definition.DoesOverrideModel())
    {
        mBody = new ModelPartSkin(this, 0, 0, &mSkinnedParts);
        mBody->addBox(-6.0f, -8.0f, -6.0f, 12, 16, 12);

        ModelPartSkin** tentacles[8] = {
            &mTentacle1, &mTentacle2, &mTentacle3, &mTentacle4,
            &mTentacle5, &mTentacle6, &mTentacle7, &mTentacle8
        };
        int i;
        for (i = 0; i < 8; ++i)
        {
            const float angle = (float)i * 0.785398163f;
            ModelPartSkin* tentacle = new ModelPartSkin(this, 48, 0, &mSkinnedParts);
            tentacle->setPos((float)std::cos(angle) * 5.0f, 15.0f,
                             (float)std::sin(angle) * 5.0f);
            tentacle->mYRot = -angle;
            tentacle->addBox(-1.0f, 0.0f, -1.0f, 2, 18, 2);
            *tentacles[i] = tentacle;
        }
        compileSkinnedDrawlist();
    }
    else
    {
        const float scale = 0.0f;
        mBody = new ModelPartSkin(this, L"body", definition, 0, scale, &mSkinnedParts);
        mTentacle1 = new ModelPartSkin(this, L"tentacle1", definition, 0, scale, &mSkinnedParts);
        mTentacle2 = new ModelPartSkin(this, L"tentacle2", definition, 0, scale, &mSkinnedParts);
        mTentacle3 = new ModelPartSkin(this, L"tentacle3", definition, 0, scale, &mSkinnedParts);
        mTentacle4 = new ModelPartSkin(this, L"tentacle4", definition, 0, scale, &mSkinnedParts);
        mTentacle5 = new ModelPartSkin(this, L"tentacle5", definition, 0, scale, &mSkinnedParts);
        mTentacle6 = new ModelPartSkin(this, L"tentacle6", definition, 0, scale, &mSkinnedParts);
        mTentacle7 = new ModelPartSkin(this, L"tentacle7", definition, 0, scale, &mSkinnedParts);
        mTentacle8 = new ModelPartSkin(this, L"tentacle8", definition, 0, scale, &mSkinnedParts);
        mBody->setYOffset(-16.0f);
        mTentacle1->setYOffset(-16.0f);
        mTentacle2->setYOffset(-16.0f);
        mTentacle3->setYOffset(-16.0f);
        mTentacle4->setYOffset(-16.0f);
        mTentacle5->setYOffset(-16.0f);
        mTentacle6->setYOffset(-16.0f);
        mTentacle7->setYOffset(-16.0f);
        mTentacle8->setYOffset(-16.0f);
        compileSkinnedDrawlist();
    }
}

SquidModel::~SquidModel()
{
}

void SquidModel::setupAnim(float, float, float ageInTicks, float, float, float,
                           boost::shared_ptr<Entity>, _SkinAdjustments)
{
    ModelPartSkin* tentacles[8] = { mTentacle1, mTentacle2, mTentacle3, mTentacle4,
                                    mTentacle5, mTentacle6, mTentacle7, mTentacle8 };
    int i;
    for (i = 0; i < 8; ++i)
        tentacles[i]->mXRot = ageInTicks;
}

void SquidModel::render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount,
                        float ageInTicks, float netHeadYaw, float headPitch, float scale,
                        bool useDisplayList)
{
    _SkinAdjustments adjustments;
    setupAnim(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, entity, adjustments);
    mBody->render(scale, useDisplayList, false);
    mTentacle1->render(scale, useDisplayList, false);
    mTentacle2->render(scale, useDisplayList, false);
    mTentacle3->render(scale, useDisplayList, false);
    mTentacle4->render(scale, useDisplayList, false);
    mTentacle5->render(scale, useDisplayList, false);
    mTentacle6->render(scale, useDisplayList, false);
    mTentacle7->render(scale, useDisplayList, false);
    mTentacle8->render(scale, useDisplayList, false);
    callSkinnedDrawlist();
}

std::wstring SquidModel::getName()
{
    return L"squid";
}
