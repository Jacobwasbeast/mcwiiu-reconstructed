#include "game/ui/fuiFile.hpp"

#include <cstring>

#include "game/client/render/TextureUtil.hpp"
#include "game/client/render/Renderer.hpp"
#include "game/ui/fui.hpp"
#include "game/ui/fuiBitmap.hpp"
#include "game/ui/fuiEdittext.hpp"
#include "game/ui/fuiFontName.hpp"
#include "game/ui/fuiImportAsset.hpp"
#include "game/ui/fuiReference.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/fuiRenderNodeBitmap.hpp"
#include "game/ui/fuiRenderNodeStage.hpp"
#include "game/ui/fuiRenderNodeTimeline.hpp"
#include "game/ui/fuiShape.hpp"
#include "game/ui/fuiShapeComponent.hpp"
#include "game/ui/fuiSymbol.hpp"
#include "game/ui/fuiTimeline.hpp"
#include "game/ui/fuiTimelineAction.hpp"
#include "game/ui/fuiTimelineEvent.hpp"
#include "game/ui/fuiTimelineEvent_version0.hpp"
#include "game/ui/fuiTimelineEventName.hpp"
#include "game/ui/fuiTimelineFrame.hpp"
#include "game/ui/fuiVert.hpp"

extern fui* gFui;

struct D3DXIMAGE_INFO
{
    unsigned int Width;
    unsigned int Height;
    unsigned int Depth;
    unsigned int MipLevels;
    unsigned int Format;
    unsigned int ResourceType;
    unsigned int ImageFileFormat;
};

fuiFile::fuiFile()
    : index(0), stage(NULL), rootNode(NULL), customDrawCallback(NULL), data(NULL)
{
}

fuiFile::~fuiFile()
{
    if (stage != NULL)
        delete stage;
    unsigned int i;
    for (i = 0; i < header.bitmapCount; ++i)
    {
        if (bitmaps[i].textureId != 0)
        {
            if (bitmaps[i].textureId > 0)
                gRenderer->TextureFree(bitmaps[i].textureId);
            gFui->removeManagedTexture(this, &bitmaps[i]);
        }
    }
    delete[] data;
}

int fuiFile::addDataRegion(unsigned int count, int stride, unsigned char** cursor, void (*swap)(void*))
{
    int first = 0;
    if (count != 0)
    {
        first = (int)*cursor;
        unsigned int i;
        for (i = 0; i < count; ++i)
        {
            swap(*cursor);
            *cursor += stride;
        }
    }
    return first;
}

unsigned int fuiFile::load(arrayWithLength<unsigned char> bytes, int value)
{
    unsigned char* source = bytes.data;
    std::memcpy(&header, source, sizeof(header));
    header.EndianSwap();
    unsigned char version = (unsigned char)header.signature;
    if (bytes.length != header.dataLength + sizeof(header))
    {
        data = NULL;
        return 0;
    }
    data = new unsigned char[header.dataLength];
    std::memcpy(data, source + sizeof(header), header.dataLength);
    unsigned char* cursor = data;
    fileIndex = value;
    timelineData = (fuiTimeline*)addDataRegion(header.timelineCount, 0x1c, &cursor, fuiTimeline::EndianSwap);
    timelineActionData = (fuiTimelineAction*)addDataRegion(header.timelineActionCount, 0x84, &cursor, fuiTimelineAction::EndianSwap);
    shapeData = (fuiShape*)addDataRegion(header.shapeCount, 0x1c, &cursor, fuiShape::EndianSwap);
    shapeComponentData = (fuiShapeComponent*)addDataRegion(header.shapeComponentCount, 0x2c, &cursor, fuiShapeComponent::EndianSwap);
    vertexData = (fuiVert*)addDataRegion(header.vertCount, 8, &cursor, fuiVert::EndianSwap);
    frameData = (fuiTimelineFrame*)addDataRegion(header.timelineFrameCount, 0x48, &cursor, fuiTimelineFrame::EndianSwap);
    if (version == 0)
    {
        eventData = (fuiTimelineEvent*)addDataRegion(header.timelineEventCount, 0x48, &cursor,
            fuiTimelineEvent_version0::EndianSwap);
        unsigned int i;
        for (i = 0; i < header.timelineEventCount; ++i)
        {
            fuiTimelineEvent_version0 event = ((fuiTimelineEvent_version0*)eventData)[i];
            eventData[i] = event;
        }
    }
    else
    {
        eventData = (fuiTimelineEvent*)addDataRegion(header.timelineEventCount, 0x48, &cursor,
            fuiTimelineEvent::EndianSwap);
    }
    eventNameData = (fuiTimelineEventName*)addDataRegion(header.timelineEventNameCount, 0x40, &cursor, fuiTimelineEventName::EndianSwap);
    referenceData = (fuiReference*)addDataRegion(header.referenceCount, 0x48, &cursor, fuiReference::EndianSwap);
    editTextData = (fuiEdittext*)addDataRegion(header.edittextCount, 0x138, &cursor, fuiEdittext::EndianSwap);
    fontNameData = (fuiFontName*)addDataRegion(header.fontNameCount, 0x104, &cursor, fuiFontName::EndianSwap);
    symbolData = (fuiSymbol*)addDataRegion(header.symbolCount, 0x48, &cursor, fuiSymbol::EndianSwap);
    importAssetData = (fuiImportAsset*)addDataRegion(header.importAssetCount, 0x40, &cursor, fuiImportAsset::EndianSwap);
    bitmaps = (fuiBitmap*)addDataRegion(header.bitmapCount, 0x20, &cursor, fuiBitmap::EndianSwap);
    imageData = cursor;
    rootNode = NULL;
    return 1;
}

unsigned int fuiFile::resolveReferences(fuiFile* other)
{
    if (referenceData == NULL)
        return 0;
    if (other->fileIndex != fileIndex || other->stage != NULL)
        return 1;
    unsigned int unresolved = 0;
    unsigned int i;
    for (i = 0; i < header.referenceCount; ++i)
    {
        if (referenceData[i].symbolIndex == (unsigned int)-1)
        {
            unsigned int j;
            for (j = 0; j < other->header.symbolCount; ++j)
            {
                if (std::strcmp(referenceData[i].name, other->symbolData[j].name) == 0)
                {
                    referenceData[i].symbolIndex = j;
                    referenceData[i].index = other->index;
                    break;
                }
            }
            if (j == other->header.symbolCount)
                unresolved = 1;
        }
    }
    return unresolved;
}

void fuiFile::dumpUnresolvedReferences()
{
    if (referenceData == NULL || header.referenceCount == 0)
        return;
}

void fuiFile::initialiseRenderTexture(fuiBitmap* bitmap, bool force)
{
    if (bitmap->textureId == 0)
    {
        int width;
        int height;
        int* textureData = NULL;
        if (bitmap->type >= eFuiBitmapType_Jpeg && bitmap->type <= eFuiBitmapType_Jpeg3)
        {
            int length = bitmap->jpegDataSize != 0 ? bitmap->jpegDataSize : bitmap->imageDataSize;
            textureData = (int*)TextureUtil::decodeJpeg(imageData + bitmap->imageDataOffset, length, &width, &height);
            if (bitmap->type == eFuiBitmapType_Jpeg3)
                TextureUtil::insertZlibAlphaChannelData(textureData, width, height,
                    imageData + bitmap->imageDataOffset + bitmap->jpegDataSize,
                    bitmap->imageDataSize - bitmap->jpegDataSize);
        }
        else
        {
            D3DXIMAGE_INFO imageInfo;
            imageInfo.Width = bitmap->width;
            imageInfo.Height = bitmap->height;
            gRenderer->LoadTextureData(imageData + bitmap->imageDataOffset, bitmap->imageDataSize,
                &imageInfo, &textureData);
        }
        if (textureData == NULL)
        {
            bitmap->textureId = -1;
            gFui->manageTexture(this, bitmap);
            return;
        }
        fui::EndianSwapImageData(textureData, bitmap->width * bitmap->height);
        bitmap->textureId = gRenderer->TextureCreate();
        if (bitmap->textureId != -1)
        {
            gRenderer->TextureBind(bitmap->textureId);
            gRenderer->TextureSetTextureLevels(1);
            gRenderer->TextureData(bitmap->width, bitmap->height, textureData, 0,
                C4JRender::eTextureFormat_RGBA8);
            delete textureData;
            textureData = NULL;
            if (force)
            {
                gRenderer->TextureSetParam(3, 0);
                gRenderer->TextureSetParam(4, 0);
            }
        }
    }
    gFui->manageTexture(this, bitmap);
}

fuiRenderNode* fuiFile::getRootNode()
{
    return rootNode;
}

int fuiFile::createNodeFromSymbol(const char* name, fuiRenderNode* parent, int value)
{
    if (fileIndex != value)
        return 0;
    unsigned int i;
    for (i = 0; i < header.symbolCount; ++i)
    {
        if (std::strcmp(symbolData[i].name, name) == 0)
        {
            if (symbolData[i].objectType == eFuiObjectType_Timeline)
            {
                fuiRenderNode* node = new fuiRenderNodeTimeline(parent->getStage(), NULL,
                    &timelineData[symbolData[i].objectIndex], 0, 0, NULL, 0, false, this);
                node->disableCtor();
                node->gather(false);
                return (int)node;
            }
            if (symbolData[i].objectType == eFuiObjectType_Bitmap)
                return (int)new fuiRenderNodeBitmap(parent->getStage(), NULL,
                    &bitmaps[symbolData[i].objectIndex], 0, 0, NULL, 0, false, this);
        }
    }
    return 0;
}

fuiRenderNode* fuiFile::findNode(const char* name)
{
    return rootNode->findNode(name);
}

void fuiFile::setVisible(const char* name, bool visible)
{
    findNode(name)->setVisibility(visible);
}

void fuiFile::setCustomDrawCallback(void (*callback)(void*, const char*, fuiRect*), void* context)
{
    customDrawCallback = callback;
    customDrawContext = context;
}

double fuiFile::getStageWidth()
{
    return (double)(header.stageRect.xMax - header.stageRect.xMin);
}

double fuiFile::getStageHeight()
{
    return (double)(header.stageRect.yMax - header.stageRect.yMin);
}

void fuiFile::setIndex(int value)
{
    index = value;
}
