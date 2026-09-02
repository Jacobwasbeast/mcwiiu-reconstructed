#ifndef GAME_CORE_CAMERACONTROLLER_HPP
#define GAME_CORE_CAMERACONTROLLER_HPP

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>

class DataInputStream;
class DataOutputStream;
class Player;
class Vec3;

class CameraController
{
public:
    class SplineNode
    {
    public:
        SplineNode(Vec3* pos, Vec3* rot, float time)
            : mPos(pos)
            , mRot(rot)
            , mTime(time)
        {
        }

        Vec3* mPos;
        Vec3* mRot;
        float mTime;
    };

    CameraController(int cameraMode);
    ~CameraController();

    void reset();
    void clear();

    void attachToPlayer(boost::shared_ptr<Player> player, bool ownedByPlayer);

    int getCameraMode();
    int getCurveType();

    std::vector<SplineNode> getNodesAndControls();
    std::vector<SplineNode>& getNodes();

    SplineNode* getPointForPos(Vec3* pos);
    void deletePointByPos(Vec3* pos);
    void addNodeAfter(Vec3* pos);
    void addNodeBefore(Vec3* pos);

    std::vector<Vec3*> GetDrawingPoints();

    static Vec3* CalculateBezierPoint(float part, Vec3* start, Vec3* startControl, Vec3* endControl, Vec3* end);
    static Vec3* CalculateBezierPointTangent(float part, Vec3* start, Vec3* startControl, Vec3* endControl, Vec3* end);

    void CalculateCurveLengths();

    std::vector<Vec3*> GetDrawingNormals();

    static double MapTimeToCurveDistance(std::vector<double>& curveLengths, double part);

    void LoadCameraPath(std::wstring fileName);
    void SaveCameraPath(std::wstring fileName);

    void insertNode(int index, double x, double y, double z, double xRot, double yRot, double zRot, float time);
    void addNode(double x, double y, double z, double xRot, double yRot, double zRot, float time);

    Vec3* getCurrentPos();
    Vec3* getCurrentRot();

    void update(float deltaTime);

    void Deserialise(DataInputStream* input);
    void Serialise(DataOutputStream* output);

    Player* mPlayer;
    std::vector<SplineNode> mNodes;
    std::vector<std::vector<double> > mCurveLengths;
    Vec3* mFromPos;
    Vec3* mFromRot;
    Vec3* mToPos;
    Vec3* mToRot;
    Vec3* mCurrentPos;
    Vec3* mCurrentRot;
    float mCurrentTime;
    float mNodeDuration;
    int mCurveType;
    int mCameraMode;
    int mSavedThirdPersonView;
    int mCurrentNode;
    int mCurrentCurve;
};

#endif
