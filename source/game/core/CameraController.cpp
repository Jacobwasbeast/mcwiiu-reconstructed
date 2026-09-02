#include "game/core/CameraController.hpp"

#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/generation/Player.hpp"

CameraController::CameraController(int cameraMode)
    : mPlayer(NULL)
    , mFromPos(NULL)
    , mFromRot(NULL)
    , mToPos(NULL)
    , mToRot(NULL)
    , mCurrentPos(Vec3::newPermanent(0.0, 0.0, 0.0))
    , mCurrentRot(Vec3::newPermanent(0.0, 0.0, 0.0))
    , mCurrentTime(0.0f)
    , mNodeDuration(0.0f)
    , mCurveType(cameraMode != 3)
    , mCameraMode(cameraMode)
    , mSavedThirdPersonView(0)
    , mCurrentNode(-1)
    , mCurrentCurve(0)
{
}

CameraController::~CameraController()
{
    clear();

    delete mCurrentPos;
    delete mCurrentRot;

    mCurrentNode = -1;
    mCurrentCurve = 0;
}

void CameraController::reset()
{
    mFromPos = NULL;
    mFromRot = NULL;
    mToPos = NULL;
    mToRot = NULL;
    mCurrentTime = 0.0f;
    mNodeDuration = 0.0f;
    mCurrentNode = -1;
    mCurrentCurve = 0;
}

void CameraController::clear()
{
    for (std::vector<SplineNode>::iterator node = mNodes.begin(); node != mNodes.end(); ++node)
    {
        delete node->mPos;
        delete node->mRot;
    }

    mNodes.clear();

    reset();
}

void CameraController::attachToPlayer(boost::shared_ptr<Player> player, bool ownedByPlayer)
{
    if (mPlayer != NULL)
    {
        mPlayer->SetThirdPersonView(mSavedThirdPersonView);
        mPlayer->SetCameraController(NULL, true);
    }

    mPlayer = player.get();
    mSavedThirdPersonView = player->GetThirdPersonView(true);
    player->SetCameraController(this, ownedByPlayer);
}

int CameraController::getCameraMode()
{
    return mCameraMode;
}

Vec3* CameraController::CalculateBezierPoint(float part, Vec3* start, Vec3* startControl, Vec3* endControl, Vec3* end)
{
    float remainder = 1.0f - part;

    return Vec3::newTemp(start)->multiply(remainder * remainder * remainder)
        ->add(Vec3::newTemp(startControl)->multiply(3.0f * remainder * remainder * part))
        ->add(Vec3::newTemp(endControl)->multiply(3.0f * remainder * (part * part)))
        ->add(Vec3::newTemp(end)->multiply(part * part * part));
}

void CameraController::CalculateCurveLengths()
{
    mCurveLengths.clear();

    Vec3* last = NULL;

    for (int node = 0; node < (int)mNodes.size() - 3; node += 3)
    {
        mCurveLengths.push_back(std::vector<double>());
        mCurveLengths.back().push_back(0.0);

        double length = 0.0;

        Vec3* start = mNodes[node].mPos;
        Vec3* startControl = mNodes[node + 1].mPos;
        Vec3* endControl = mNodes[node + 2].mPos;
        Vec3* end = mNodes[node + 3].mPos;

        if (node == 0)
        {
            last = CalculateBezierPoint(0.0f, start, startControl, endControl, end);
        }

        for (int step = 1; step <= 100; ++step)
        {
            Vec3* point = CalculateBezierPoint((float)step / 100.0f, start, startControl, endControl, end);

            length += last->vectorTo(point)->length();

            mCurveLengths.back().push_back(length);

            last = point;
        }
    }
}

double CameraController::MapTimeToCurveDistance(std::vector<double>& curveLengths, double part)
{
    double distance = part * curveLengths[100];

    int low = 0;
    int high = 100;
    int step = 0;

    while (low < high)
    {
        step = low + (high - low) / 2;

        if (curveLengths[step] < distance)
        {
            low = step + 1;
        }
        else
        {
            high = step;
        }
    }

    if (distance < curveLengths[step])
    {
        --step;
    }

    double mapped;

    if (curveLengths[step] == distance)
    {
        mapped = step;
    }
    else
    {
        mapped = step + (distance - curveLengths[step]) / (curveLengths[step + 1] - curveLengths[step]);
    }

    return mapped / 100.0;
}

void CameraController::insertNode(int index, double x, double y, double z, double xRot, double yRot, double zRot, float time)
{
    Vec3* pos = Vec3::newPermanent(x, y, z);

    if (index == 0)
    {
        mNodes.insert(mNodes.begin(), SplineNode(pos, Vec3::newPermanent(xRot, yRot, zRot), time));

        Vec3* control = Vec3::newPermanent(pos->add(mPlayer->calculateViewVector((float)xRot, (float)yRot)->multiply(4.0f)));

        mNodes.insert(mNodes.begin() + 1, SplineNode(control, Vec3::newPermanent(0.0, 0.0, 0.0), -1.0f));

        if (mNodes.size() > 2)
        {
            double length = mNodes[2].mPos->vectorTo(mNodes[0].mPos)->length() * 0.25;

            if (length > 4.0)
            {
                length = 4.0;
            }

            Vec3* nextControl = Vec3::newPermanent(mNodes[2].mPos->add(mNodes[3].mPos->vectorTo(mNodes[2].mPos)->normalize()->multiply(length)));

            mNodes.insert(mNodes.begin() + 2, SplineNode(nextControl, Vec3::newPermanent(0.0, 0.0, 0.0), -1.0f));
        }
    }
    else
    {
        Vec3* direction = pos->vectorTo(mNodes[index - 2].mPos);

        double length = direction->length() * 0.25;

        if (length > 4.0)
        {
            length = 4.0;
        }

        Vec3* beforeControl = Vec3::newPermanent(pos->add(direction->normalize()->multiply(length)));
        Vec3* afterControl = Vec3::newPermanent(pos->add(beforeControl->vectorTo(pos)->normalize()->multiply(length)));

        mNodes.insert(mNodes.begin() + index, SplineNode(beforeControl, Vec3::newPermanent(0.0, 0.0, 0.0), -1.0f));
        mNodes.insert(mNodes.begin() + index + 1, SplineNode(pos, Vec3::newPermanent(xRot, yRot, zRot), time));
        mNodes.insert(mNodes.begin() + index + 2, SplineNode(afterControl, Vec3::newPermanent(0.0, 0.0, 0.0), -1.0f));
    }

    CalculateCurveLengths();
}

void CameraController::addNode(double x, double y, double z, double xRot, double yRot, double zRot, float time)
{
    if (mCurveType == 0)
    {
        mNodes.push_back(SplineNode(Vec3::newPermanent(x, y, z), Vec3::newPermanent(xRot, yRot, zRot), time));
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

void CameraController::update(float deltaTime)
{
    if (mCurveType == 0)
    {
        if (mNodes.size() > 1)
        {
            if (mCurrentNode == -1)
            {
                mFromPos = mNodes[0].mPos;
                mFromRot = mNodes[0].mRot;
                mToPos = mNodes[1].mPos;
                mToRot = mNodes[1].mRot;
                mNodeDuration = mNodes[1].mTime;
                mCurrentNode = 2;
            }

            float part = (mCurrentTime + deltaTime) / mNodeDuration;

            part = part * part * (3.0f - (part + part));

            if (1.0f - part < 0.0f)
            {
                part = 1.0f;
            }

            mCurrentTime += deltaTime;

            *mCurrentPos = *mFromPos->interpolateTo(mToPos, part);
            *mCurrentRot = *mFromRot->interpolateTo(mToRot, part);

            if (mCurrentTime > mNodeDuration)
            {
                if (mCurrentNode < (int)mNodes.size())
                {
                    mFromPos = mToPos;
                    mFromRot = mToRot;
                    mToPos = mNodes[mCurrentNode].mPos;
                    mToRot = mNodes[mCurrentNode].mRot;
                    mNodeDuration = mNodes[mCurrentNode].mTime;
                    mCurrentTime = 0.0f;
                    ++mCurrentNode;
                }
                else
                {
                    reset();

                    if (mPlayer != NULL)
                    {
                        mPlayer->SetThirdPersonView(mSavedThirdPersonView);
                        mPlayer->SetCameraController(NULL, true);
                    }
                }
            }
        }
    }
    else if (mCurveType == 1)
    {
        if (mNodes.size() > 1)
        {
            if (mCurrentNode == -1)
            {
                mCurrentNode = 0;
            }

            float time = mCurrentTime + deltaTime;
            double distance = time * 0.2;

            while (mCurveLengths[mCurrentCurve][100] < distance
                && mCurrentNode < (int)mNodes.size() - 4
                && mCurrentCurve < mCurveLengths.size())
            {
                distance -= mCurveLengths[mCurrentCurve][100];

                ++mCurrentCurve;
                mCurrentNode += 3;

                time = (float)(distance / 0.2);
            }

            mCurrentTime = time;

            if (mCurrentNode < (int)mNodes.size() - 4)
            {
                double part = distance / mCurveLengths[mCurrentCurve][100];

                mCurrentPos->set(CalculateBezierPoint((float)MapTimeToCurveDistance(mCurveLengths[mCurrentCurve], part),
                    mNodes[mCurrentNode].mPos,
                    mNodes[mCurrentNode + 1].mPos,
                    mNodes[mCurrentNode + 2].mPos,
                    mNodes[mCurrentNode + 3].mPos));

                *mCurrentRot = *mNodes[mCurrentNode].mRot->interpolateTo(mNodes[mCurrentNode + 3].mRot, part);
            }
            else
            {
                reset();

                if (mPlayer != NULL)
                {
                    mPlayer->SetThirdPersonView(mSavedThirdPersonView);
                    mPlayer->SetCameraController(NULL, true);
                }
            }
        }
    }
}

void CameraController::Deserialise(DataInputStream* input)
{
    int version = input->readInt();

    if (version < 1)
    {
        int count = input->readInt();

        for (int node = 0; node < count; ++node)
        {
            double x = input->readDouble();
            double y = input->readDouble();
            double z = input->readDouble();
            double xRot = input->readDouble();
            double yRot = input->readDouble();
            double zRot = input->readDouble();

            mNodes.push_back(SplineNode(Vec3::newPermanent(x, y, z), Vec3::newPermanent(xRot, yRot, zRot), -1.0f));
        }

        CalculateCurveLengths();
    }
}

void CameraController::Serialise(DataOutputStream* output)
{
    output->writeInt(0);
    output->writeInt((int)mNodes.size());

    for (std::vector<SplineNode>::iterator node = mNodes.begin(); node != mNodes.end(); ++node)
    {
        output->writeDouble(node->mPos->x);
        output->writeDouble(node->mPos->y);
        output->writeDouble(node->mPos->z);
        output->writeDouble(node->mRot->x);
        output->writeDouble(node->mRot->y);
        output->writeDouble(node->mRot->z);
    }
}
