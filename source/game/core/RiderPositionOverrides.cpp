#include "game/core/RiderPositionOverrides.hpp"

#include <cmath>

#include <boost/unordered_map.hpp>

#include "game/core/DataInputStream.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/EntityIO.hpp"
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
        overridePosition.isSitting = stream->readByte();
        overridePosition.isSaddled = stream->readByte();
        overridePosition.x = stream->readDouble();
        overridePosition.y = stream->readDouble();
        overridePosition.z = stream->readDouble();
        gRiderPositionOverrides[EntityIO::byString(entityName)].push_back(overridePosition);

        overridePosition.isSitting = stream->readByte();
        overridePosition.isSaddled = stream->readByte();
        overridePosition.x = stream->readDouble();
        overridePosition.y = stream->readDouble();
        overridePosition.z = stream->readDouble();
        gRiderPositionOverrides[EntityIO::byString(entityName)].push_back(overridePosition);
    }

    if ((count & 1) != 0)
    {
        PositionOverrideStruct overridePosition;
        overridePosition.isSitting = stream->readByte();
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
    unsigned long long entityType = vehicle->GetType();
    boost::unordered_map<unsigned long long, std::vector<PositionOverrideStruct> >::iterator it =
        gRiderPositionOverrides.find(entityType);
    if (it == gRiderPositionOverrides.end())
        return false;

    unsigned char isSitting = 0;
    unsigned char isSaddled = 0;

    std::vector<PositionOverrideStruct>& overrides = it->second;
    for (unsigned int i = 0; i < overrides.size(); ++i)
    {
        PositionOverrideStruct& overridePosition = overrides.at(i);
        if (overridePosition.isSitting != isSitting ||
            overridePosition.isSaddled != isSaddled)
            continue;

        Vec3* riderOffset = Vec3::newTemp(
            overridePosition.x,
            overridePosition.y + passenger->getRidingHeight(),
            overridePosition.z);
        riderOffset->yRot(-(vehicle->mYRot * 0.01745329238474369f));

        Vec3* vehiclePosition = Vec3::newTemp(
            vehicle->mX,
            vehicle->mBoundingBox->mMinY,
            vehicle->mZ);

        double cosine = 1.0;
        double sine = 1.0;
        if ((vehicle->GetType() & 0x000000000C000000ULL) ==
            0x000000000C000000ULL)
        {
            float bodyRotation = vehicle->mYRot * 0.01745329238474369f;
            cosine = Mth::cos(bodyRotation);
            sine = Mth::sin(bodyRotation);
        }

        passenger->setPos(
            riderOffset->x * sine + vehiclePosition->x,
            riderOffset->y + vehiclePosition->y,
            riderOffset->z * cosine + vehiclePosition->z);
        return true;
    }

    return false;
}
