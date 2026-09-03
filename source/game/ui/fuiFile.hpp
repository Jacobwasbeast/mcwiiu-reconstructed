#ifndef GAME_UI_FUIFILE_HPP
#define GAME_UI_FUIFILE_HPP
#include "game/world/biome/arrayWithLength.hpp"
class fuiBitmap;
class fuiRenderNode;
class fuiRect;
struct fuiFile
{
    unsigned int header[38];
    int index;
    int fileIndex;
    int timelineData;
    int timelineActionData;
    int shapeData;
    int shapeComponentData;
    int vertexData;
    int frameData;
    int eventData;
    int eventNameData;
    int referenceData;
    int editTextData;
    int fontNameData;
    int symbolData;
    int importAssetData;
    unsigned char* data;
    fuiBitmap* bitmaps;
    int bitmapCount;
    fuiRenderNode* rootNode;
    void* customDrawCallback;
    void* customDrawContext;
    int ownsData;
    fuiFile();
    ~fuiFile();
    int addDataRegion(unsigned int count, int stride, unsigned char** cursor, void (*swap)(void*));
    unsigned int load(arrayWithLength<unsigned char> bytes, int fileIndex);
    unsigned int resolveReferences(fuiFile* other);
    void dumpUnresolvedReferences();
    void initialiseRenderTexture(fuiBitmap* bitmap, bool force);
    fuiRenderNode* getRootNode();
    int createNodeFromSymbol(const char* name, fuiRenderNode* parent, int fileIndex);
    fuiRenderNode* findNode(const char* name);
    void setVisible(const char* name, bool visible);
    void setCustomDrawCallback(void (*callback)(void*, const char*, fuiRect*), void* context);
    double getStageWidth();
    double getStageHeight();
    void setIndex(int value);
    void play(const char* name);
    void stop(const char* name);
    void gotoFrame(const char* name, int frame);
    void gotoFrameAndPlay(const char* name, int frame);
    void gotoFrameAndStop(const char* name, int frame);
};
#endif
