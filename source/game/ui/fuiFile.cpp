#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiBitmap.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include <cstring>
namespace fui
{
void manageTexture(fuiFile*, fuiBitmap*);
void removeManagedTexture(fuiFile*, fuiBitmap*);
}
static void swapBytes(void*) {}
fuiFile::fuiFile()
    : index(0), fileIndex(0), timelineData(0), timelineActionData(0), shapeData(0), shapeComponentData(0), vertexData(0), frameData(0), eventData(0), eventNameData(0), referenceData(0), editTextData(0), fontNameData(0), symbolData(0), importAssetData(0), data(NULL), bitmaps(NULL), bitmapCount(0), rootNode(NULL), customDrawCallback(NULL), customDrawContext(NULL), ownsData(0)
{
}
fuiFile::~fuiFile()
{
    if (rootNode != NULL)
        delete rootNode;
    if (bitmaps != NULL)
    {
        unsigned int i;
        for (i = 0; i < (unsigned int)bitmapCount; ++i)
        {
            if (bitmaps[i].textureId < 0 || bitmaps[i].textureId > 0)
                fui::removeManagedTexture(this, &bitmaps[i]);
        }
    }
    delete[] data;
}
int fuiFile::addDataRegion(unsigned int count, int stride, unsigned char** cursor, void (*swap)(void*))
{
    int first = 0;
    if (count != 0 && cursor != NULL && *cursor != NULL)
    {
        first = (int)(*cursor);
        while (count != 0)
        {
            if (swap != NULL)
                swap(*cursor);
            *cursor += stride;
            --count;
        }
    }
    return first;
}
unsigned int fuiFile::load(arrayWithLength<unsigned char> bytes, int value)
{
    if (bytes.data == NULL || bytes.length < 0x98)
    {
        data = NULL;
        return 0;
    }
    std::memcpy(header, bytes.data, 0x98);
    fileIndex = value;
    unsigned int length = header[2];
    if (bytes.length != length + 0x98)
    {
        data = NULL;
        return 0;
    }
    data = new unsigned char[length];
    std::memcpy(data, bytes.data + 0x98, length);
    unsigned char* cursor = data;
    timelineData = addDataRegion(header[19], 0x1c, &cursor, swapBytes);
    timelineActionData = addDataRegion(header[21], 0x84, &cursor, swapBytes);
    shapeData = addDataRegion(header[22], 0x1c, &cursor, swapBytes);
    shapeComponentData = addDataRegion(header[23], 0x2c, &cursor, swapBytes);
    vertexData = addDataRegion(header[24], 8, &cursor, swapBytes);
    frameData = addDataRegion(header[25], 0x48, &cursor, swapBytes);
    eventData = addDataRegion(header[26], 0x48, &cursor, swapBytes);
    eventNameData = addDataRegion(header[20], 0x40, &cursor, swapBytes);
    referenceData = addDataRegion(header[27], 0x48, &cursor, swapBytes);
    editTextData = addDataRegion(header[28], 0x138, &cursor, swapBytes);
    fontNameData = addDataRegion(header[32], 0x104, &cursor, swapBytes);
    symbolData = addDataRegion(header[29], 0x48, &cursor, swapBytes);
    importAssetData = addDataRegion(header[33], 0x40, &cursor, swapBytes);
    bitmaps = (fuiBitmap*)addDataRegion(header[30], 0x20, &cursor, swapBytes);
    bitmapCount = header[30];
    rootNode = NULL;
    return 1;
}
unsigned int fuiFile::resolveReferences(fuiFile* other)
{
    if (referenceData == 0)
        return 0;
    if (other == NULL || other->fileIndex != fileIndex || other->rootNode != NULL)
        return 1;
    return 0;
}
void fuiFile::dumpUnresolvedReferences()
{
    if (referenceData == 0 || header[27] == 0)
        return;
}
void fuiFile::initialiseRenderTexture(fuiBitmap* bitmap, bool force)
{
    if (bitmap == NULL)
        return;
    if (bitmap->textureId == 0 || force)
        fui::manageTexture(this, bitmap);
}
fuiRenderNode* fuiFile::getRootNode()
{
    return rootNode;
}
int fuiFile::createNodeFromSymbol(const char* name, fuiRenderNode* parent, int value)
{
    if (name == NULL || value != fileIndex || symbolData == 0)
        return 0;
    return parent != NULL ? 1 : 0;
}
fuiRenderNode* fuiFile::findNode(const char* name)
{
    if (rootNode == NULL)
        return NULL;
    return rootNode->findNode(name);
}
void fuiFile::setVisible(const char* name, bool visible)
{
    fuiRenderNode* node = findNode(name);
    if (node != NULL)
        node->setVisibility(visible);
}
void fuiFile::setCustomDrawCallback(void (*callback)(void*, const char*, fuiRect*), void* context)
{
    customDrawCallback = (void*)callback;
    customDrawContext = context;
}
double fuiFile::getStageWidth()
{
    return (double)(*(float*)&header[35] - *(float*)&header[34]);
}
double fuiFile::getStageHeight()
{
    return (double)(*(float*)&header[37] - *(float*)&header[36]);
}
void fuiFile::setIndex(int value)
{
    index = value;
}
void fuiFile::play(const char* name)
{
    fuiRenderNode* node = findNode(name);
    if (node != NULL)
        node->asTimeline();
}
void fuiFile::stop(const char* name)
{
    fuiRenderNode* node = findNode(name);
    if (node != NULL)
        node->asTimeline();
}
void fuiFile::gotoFrame(const char* name, int frame)
{
    fuiRenderNode* node = findNode(name);
    if (node != NULL)
        node->setX((float)frame);
}
void fuiFile::gotoFrameAndPlay(const char* name, int frame)
{
    gotoFrame(name, frame);
    play(name);
}
void fuiFile::gotoFrameAndStop(const char* name, int frame)
{
    gotoFrame(name, frame);
    stop(name);
}
