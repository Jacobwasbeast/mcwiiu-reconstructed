#include "game/core/CameraController.hpp"

#include <algorithm>

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/world/generation/Player.hpp"

CameraController::CameraController(int cameraMode)
{
    mCameraMode = cameraMode;
    mPlayerCameraMode = 0;
    mPlayer = NULL;
    mFromPos = NULL;
    mFromRot = NULL;
    mToPos = NULL;
    mToRot = NULL;
    mCurveType = cameraMode != 3;
    mCurrentPos = Vec3::newPermanent(0.0, 0.0, 0.0);
    mCurrentRot = Vec3::newPermanent(0.0, 0.0, 0.0);
    mCurveIndex = 0;
    mCurrentNode = -1;
    mTime = 0.0f;
    mDuration = 0.0f;
}

CameraController::~CameraController()
{
    clear();
    delete mCurrentPos;
    delete mCurrentRot;
}

void CameraController::reset()
{
    mFromPos = NULL;
    mFromRot = NULL;
    mToPos = NULL;
    mToRot = NULL;
    mCurrentNode = -1;
    mTime = 0.0f;
    mDuration = 0.0f;
    mCurveIndex = 0;
}

void CameraController::clear()
{
    std::vector<SplineNode>::iterator it;
    for (it = mNodes.begin(); it != mNodes.end(); ++it)
    {
        delete it->position;
        delete it->rotation;
    }
    mNodes.clear();
    reset();
}

void CameraController::attachToPlayer(boost::shared_ptr<Player> player, bool active)
{
    if (mPlayer != NULL)
    {
        mPlayer->SetThirdPersonView(mPlayerCameraMode);
        mPlayer->SetCameraController(NULL, true);
    }
    mPlayer = player.get();
    mPlayerCameraMode = player->GetThirdPersonView(true);
    player->SetCameraController(this, active);
}

int CameraController::getCameraMode()
{
    return mCameraMode;
}

Vec3* CameraController::CalculateBezierPoint(float t, Vec3* p0, Vec3* p1, Vec3* p2, Vec3* p3)
{
    float inverse = 1.0f - t;
    Vec3* result = Vec3::newTemp(p0)->multiply(inverse * inverse * inverse);
    result = result->add(Vec3::newTemp(p1)->multiply(3.0f * inverse * inverse * t));
    result = result->add(Vec3::newTemp(p2)->multiply(3.0f * inverse * t * t));
    result = result->add(Vec3::newTemp(p3)->multiply(t * t * t));
    return result;
}

void CameraController::CalculateCurveLengths()
{
    mCurveLengths.clear();
    int count = (int)mNodes.size();
    int segment;
    for (segment = 0; segment + 3 < count; segment += 3)
    {
        std::vector<double> lengths;
        double total = 0.0;
        Vec3* previous = CalculateBezierPoint(0.0f, mNodes[segment].position,
                                               mNodes[segment + 1].position,
                                               mNodes[segment + 2].position,
                                               mNodes[segment + 3].position);
        lengths.push_back(0.0);
        int sample;
        for (sample = 1; sample <= 100; ++sample)
        {
            float t = (float)sample / 100.0f;
            Vec3* point = CalculateBezierPoint(t, mNodes[segment].position,
                                                mNodes[segment + 1].position,
                                                mNodes[segment + 2].position,
                                                mNodes[segment + 3].position);
            total += previous->vectorTo(point)->length();
            lengths.push_back(total);
            previous = point;
        }
        mCurveLengths.push_back(lengths);
    }
}

double CameraController::MapTimeToCurveDistance(std::vector<double>& curveLengths, double time)
{
    double target = time * curveLengths[100];
    int low = 0;
    int high = 100;
    int mid = 0;
    while (low < high)
    {
        mid = low + (high - low) / 2;
        if (curveLengths[mid] < target)
            low = mid + 1;
        else
            high = mid;
    }
    if (target < curveLengths[mid])
        --mid;
    if (curveLengths[mid] == target)
        return (double)mid / 100.0;
    return ((double)mid + (target - curveLengths[mid]) /
            (curveLengths[mid + 1] - curveLengths[mid])) / 100.0;
}

void CameraController::insertNode(int index, double x, double y, double z, double xRot, double yRot, double zRot, float time)
{
    Vec3* position = Vec3::newPermanent(x, y, z);
    Vec3* rotation = Vec3::newPermanent(xRot, yRot, zRot);
    if (index == 0)
    {
        SplineNode node = { position, rotation, time };
        mNodes.insert(mNodes.begin(), node);
        Vec3* control = position->add(mPlayer->calculateViewVector((float)xRot, (float)yRot)->multiply(4.0f));
        SplineNode controlNode = { Vec3::newPermanent(control), Vec3::newPermanent(0.0, 0.0, 0.0), -1.0f };
        mNodes.insert(mNodes.begin() + 1, controlNode);
        if (mNodes.size() > 2)
        {
            Vec3* nextPosition = mNodes[2].position;
            double controlLength = nextPosition->vectorTo(mNodes[0].position)->length() * 0.2;
            if (controlLength > 0.25)
                controlLength = 0.25;
            Vec3* nextControl = nextPosition->add(mNodes[3].position->vectorTo(nextPosition)->normalize()->multiply(controlLength));
            SplineNode nextControlNode = { Vec3::newPermanent(nextControl), Vec3::newPermanent(0.0, 0.0, 0.0), -1.0f };
            mNodes.insert(mNodes.begin() + 2, nextControlNode);
        }
        CalculateCurveLengths();
        return;
    }
    Vec3* direction = position->vectorTo(mNodes[index - 2].position);
    double controlLength = direction->length() * 0.2;
    if (controlLength > 0.25)
        controlLength = 0.25;
    Vec3* firstControl = position->add(direction->normalize()->multiply(controlLength));
    Vec3* secondDirection = firstControl->vectorTo(position)->normalize()->multiply(controlLength);
    Vec3* secondControl = position->add(secondDirection);
    SplineNode firstControlNode = { Vec3::newPermanent(firstControl), Vec3::newPermanent(0.0, 0.0, 0.0), -1.0f };
    SplineNode node = { position, rotation, time };
    SplineNode secondControlNode = { Vec3::newPermanent(secondControl), Vec3::newPermanent(0.0, 0.0, 0.0), -1.0f };
    mNodes.insert(mNodes.begin() + index, firstControlNode);
    mNodes.insert(mNodes.begin() + index + 1, node);
    mNodes.insert(mNodes.begin() + index + 2, secondControlNode);
    CalculateCurveLengths();
}

void CameraController::addNode(double x, double y, double z, double xRot, double yRot, double zRot, float time)
{
    if (mCurveType == 0)
    {
        SplineNode node = { Vec3::newPermanent(x, y, z), Vec3::newPermanent(xRot, yRot, zRot), time };
        mNodes.push_back(node);
    }
    else if (mCurveType == 1)
    {
        insertNode((int)mNodes.size(), x, y, z, xRot, yRot, zRot, time);
    }
}

Vec3* CameraController::getCurrentPos()
{
    return mCurrentPos;
}

Vec3* CameraController::getCurrentRot()
{
    return mCurrentRot;
}

void CameraController::update(float delta)
{
    if (mCurveType == 0)
    {
        if (mNodes.size() <= 1)
            return;
        if (mCurrentNode == -1)
        {
            mFromPos = mNodes[0].position;
            mFromRot = mNodes[0].rotation;
            mToPos = mNodes[1].position;
            mToRot = mNodes[1].rotation;
            mDuration = mNodes[1].time;
            mCurrentNode = 2;
        }
        mTime += delta;
        float part = mTime / mDuration;
        double blend = part * part * (3.0 - 2.0 * part);
        if (blend > 1.0)
            blend = 1.0;
        mCurrentPos->set(mFromPos->interpolateTo(mToPos, blend));
        mCurrentRot->set(mFromRot->interpolateTo(mToRot, blend));
        if (mTime <= mDuration)
            return;
        if (mCurrentNode < (int)mNodes.size())
        {
            mFromPos = mToPos;
            mFromRot = mToRot;
            mDuration = mNodes[mCurrentNode].time;
            mToPos = mNodes[mCurrentNode].position;
            mToRot = mNodes[mCurrentNode].rotation;
            mTime = 0.0f;
            ++mCurrentNode;
            return;
        }
        reset();
        if (mPlayer != NULL)
        {
            mPlayer->SetThirdPersonView(mPlayerCameraMode);
            mPlayer->SetCameraController(NULL, true);
        }
    }
    else if (mCurveType == 1 && mNodes.size() > 1 && !mCurveLengths.empty())
    {
        if (mCurrentNode == -1)
            mCurrentNode = 0;
        mTime += delta;
        double distance = (double)mTime * 0.2;
        double curveLength = mCurveLengths[mCurveIndex][100];
        while (curveLength < distance)
        {
            if (mCurrentNode >= (int)mNodes.size() - 4 || mCurveIndex + 1 >= (int)mCurveLengths.size())
                break;
            distance -= curveLength;
            ++mCurveIndex;
            mCurrentNode += 3;
            mTime = (float)(distance / 0.2);
            curveLength = mCurveLengths[mCurveIndex][100];
        }
        if (mCurrentNode < (int)mNodes.size() - 4)
        {
            double part = distance / curveLength;
            double curvePosition = MapTimeToCurveDistance(mCurveLengths[mCurveIndex], part);
            mCurrentPos->set(CalculateBezierPoint((float)curvePosition,
                mNodes[mCurrentNode].position,
                mNodes[mCurrentNode + 1].position,
                mNodes[mCurrentNode + 2].position,
                mNodes[mCurrentNode + 3].position));
            mCurrentRot->set(mNodes[mCurrentNode + 1].rotation->interpolateTo(
                mNodes[mCurrentNode + 3].rotation, part));
            return;
        }
        reset();
        if (mPlayer != NULL)
        {
            mPlayer->SetThirdPersonView(mPlayerCameraMode);
            mPlayer->SetCameraController(NULL, true);
        }
    }
}

void CameraController::Deserialise(DataInputStream* stream)
{
    int count = stream->readInt();
    if (count < 1)
    {
        count = stream->readInt();
        int i;
        for (i = 0; i < count; ++i)
        {
            double x = stream->readDouble();
            double y = stream->readDouble();
            double z = stream->readDouble();
            double xRot = stream->readDouble();
            double yRot = stream->readDouble();
            double zRot = stream->readDouble();
            SplineNode node = { Vec3::newPermanent(x, y, z), Vec3::newPermanent(xRot, yRot, zRot), 0.5f };
            mNodes.push_back(node);
        }
        CalculateCurveLengths();
    }
}

void CameraController::Serialise(DataOutputStream* stream)
{
    stream->writeInt(0);
    stream->writeInt((int)mNodes.size());
    std::vector<SplineNode>::const_iterator it;
    for (it = mNodes.begin(); it != mNodes.end(); ++it)
    {
        stream->writeDouble(it->position->x);
        stream->writeDouble(it->position->y);
        stream->writeDouble(it->position->z);
        stream->writeDouble(it->rotation->x);
        stream->writeDouble(it->rotation->y);
        stream->writeDouble(it->rotation->z);
    }
}
