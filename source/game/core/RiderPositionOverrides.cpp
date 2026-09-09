#include "game/core/RiderPositionOverrides.hpp"

#include <cmath>

#include <boost/unordered_map.hpp>

#include "game/core/DataInputStream.hpp"
#include "game/core/Pig.hpp"
#include "game/entity/mob/AbstractHorse.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/EntityIO.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/TamableAnimal.hpp"
#include "game/core/Mth.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Vec3.hpp"

static boost::unordered_map<unsigned long long, std::vector<PositionOverrideStruct> >
    gRiderPositionOverrides;

void RiderPositionOverrides::ClearOverrides()
{
    gRiderPositionOverrides.clear();
}

void RiderPositionOverrides::ReadOverrides(DataInputStream* stream)
{
    std::wstring entityName = stream->readUTF();
    int count = stream->readInt();

    for (int i = 0; i < count / 2; ++i)
    {
        PositionOverrideStruct overridePosition;
        overridePosition.isTamed = stream->readByte();
        overridePosition.isSaddled = stream->readByte();
        overridePosition.x = stream->readDouble();
        overridePosition.y = stream->readDouble();
        overridePosition.z = stream->readDouble();
        gRiderPositionOverrides[EntityIO::byString(entityName)].push_back(overridePosition);

        overridePosition.isTamed = stream->readByte();
        overridePosition.isSaddled = stream->readByte();
        overridePosition.x = stream->readDouble();
        overridePosition.y = stream->readDouble();
        overridePosition.z = stream->readDouble();
        gRiderPositionOverrides[EntityIO::byString(entityName)].push_back(overridePosition);
    }

    if ((count & 1) != 0)
    {
        PositionOverrideStruct overridePosition;
        overridePosition.isTamed = stream->readByte();
        overridePosition.isSaddled = stream->readByte();
        overridePosition.x = stream->readDouble();
        overridePosition.y = stream->readDouble();
        overridePosition.z = stream->readDouble();
        gRiderPositionOverrides[EntityIO::byString(entityName)].push_back(overridePosition);
    }
}

bool RiderPositionOverrides::OverridePosition(
    const boost::shared_ptr<Entity>& vehicle,
    const boost::shared_ptr<Entity>& passenger)
{
    if (gRiderPositionOverrides.find(vehicle->GetType()) == gRiderPositionOverrides.end())
    {
        return false;
    }

    unsigned char isTamed = 0;
    unsigned char isSaddled = 0;
    if ((vehicle->GetType() & 0x000000000D6C0000ULL) == 0x000000000D6C0000ULL)
    {
        isTamed = boost::static_pointer_cast<TamableAnimal>(vehicle)->isTame();
    }
    else if ((vehicle->GetType() & 0x000000000D690000ULL) == 0x000000000D690000ULL)
    {
        boost::shared_ptr<AbstractHorse> horse = boost::static_pointer_cast<AbstractHorse>(vehicle);
        isSaddled = horse->isSaddled();
        isTamed = horse->isTamed();
    }
    else if (vehicle->GetType() == 0x000000002D680003ULL)
    {
        isSaddled = boost::static_pointer_cast<Pig>(vehicle)->hasSaddle();
    }

    for (unsigned int i = 0; i < gRiderPositionOverrides.at(vehicle->GetType()).size(); ++i)
    {
        PositionOverrideStruct& overridePosition =
            gRiderPositionOverrides.at(vehicle->GetType()).at(i);
        if (overridePosition.isTamed != isTamed || overridePosition.isSaddled != isSaddled)
        {
            continue;
        }

        Vec3* riderOffset = Vec3::newTemp(
            overridePosition.x,
            overridePosition.y + passenger->getRidingHeight(),
            overridePosition.z);
        riderOffset->yRot(-(vehicle->mYRot * 0.01745329238474369f));

        Vec3* vehiclePosition = Vec3::newTemp(
            vehicle->mX,
            vehicle->mBoundingBox->mMinY,
            vehicle->mZ);

        double cosine;
        double sine;
        if ((vehicle->GetType() & 0x000000000C000000ULL) == 0x000000000C000000ULL)
        {
            float bodyRotation =
                boost::static_pointer_cast<LivingEntity>(vehicle)->mYBodyRot * 0.01745329238474369f;
            cosine = Mth::cos(bodyRotation);
            sine = Mth::sin(bodyRotation);
        }
        else
        {
            cosine = 1.0f;
            sine = cosine;
        }

        passenger->setPos(
            riderOffset->x * sine + vehiclePosition->x,
            vehiclePosition->y + riderOffset->y,
            riderOffset->z * cosine + vehiclePosition->z);
        return true;
    }

    return false;
}
