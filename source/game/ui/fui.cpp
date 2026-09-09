#include "game/ui/fui.hpp"

#include <cstring>

#include "game/client/render/Renderer.hpp"
#include "game/core/System.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/FJ_KeyboardEvent.hpp"
#include "game/ui/FJ_MouseMoveEvent.hpp"
#include "game/ui/fuiBitmap.hpp"
#include "game/ui/fuiBitmapFont.hpp"
#include "game/ui/fuiImportAsset.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/fuiRenderNodeStage.hpp"
#include "game/ui/fuiRenderNodeTimeline.hpp"
#include "game/ui/fuiTimeline.hpp"
#include "game/ui/fuiTimelineAction.hpp"
#include "game/ui/fuiTTFFontSet.hpp"

fui* gFui = new fui();

fui::fui()
{
    m_matrixStack[0].scaleX = 1.0f;
    m_matrixStack[0].rotateSkew0 = 0.0f;
    m_matrixStack[0].rotateSkew1 = 0.0f;
    m_matrixStack[0].scaleY = 1.0f;
    m_matrixStack[0].translateX = 0.0f;
    m_matrixStack[0].translateY = 0.0f;
    m_colorStack[0].redAdd = 0.0f;
    m_colorStack[0].greenAdd = 0.0f;
    m_colorStack[0].blueAdd = 0.0f;
    m_colorStack[0].alphaAdd = 0.0f;
    m_colorStack[0].redMultiply = 1.0f;
    m_colorStack[0].greenMultiply = 1.0f;
    m_colorStack[0].blueMultiply = 1.0f;
    m_colorStack[0].alphaMultiply = 1.0f;
    m_matrixDepth = 0;
    m_colorDepth = 0;
    m_filterStack[0] = 0;
    m_filterDepth = 0;
    m_originX = 0.0f;
    m_originY = 0.0f;
    m_stencilMask = 1;
    for (int i = 0; i < 64; ++i)
        m_permanentFileIndices[i] = -1;
    m_frameCount = 0;
    m_enterFrameDispatch = false;
    m_lastTime = System::processTimeInMilliSecs();
    m_customTextureFunction = NULL;
    m_releaseCustomTextureFunction = NULL;
    m_resolution = 0;
    m_textureManager = new fuiTextureManager();
    InitializeCriticalSection(&m_mutex);
}

void fui::removeManagedTexture(fuiFile* file, fuiBitmap* bitmap)
{
    m_textureManager->removeManagedTexture(file, bitmap);
}

void fui::EndianSwapImageData(void* data, unsigned int length)
{
    unsigned int* words = static_cast<unsigned int*>(data);
    unsigned int i;
    for (i = 0; i < length; ++i)
    {
        unsigned int value = words[i];
        words[i] = (value << 24) | ((value & 0xff00) << 8) | (value >> 24) | ((value >> 8) & 0xff00);
    }
}

void fui::manageTexture(fuiFile* file, fuiBitmap* bitmap)
{
    m_textureManager->manageTexture(file, bitmap);
}

int fui::allocatePermanentFileIndex(int fileIndex)
{
    int* slots = m_permanentFileIndices;
    int i;
    for (i = 0; i < 64; ++i)
    {
        if (slots[i] == -1)
        {
            slots[i] = fileIndex;
            return i;
        }
    }
    return -1;
}

int fui::load(arrayWithLength<unsigned char> bytes, bool createRoot, int fileIndex)
{
    fuiFile* file = new fuiFile();
    if (file->load(bytes, fileIndex) == 0)
    {
        delete file;
        return 0;
    }

    EnterCriticalSection(&m_mutex);
    file->setIndex(allocatePermanentFileIndex(static_cast<int>(m_files.size())));

    unsigned int unresolvedReferences = file->header.importAssetCount != 0;
    unsigned int importIndex;
    for (importIndex = 0; importIndex < file->header.importAssetCount && unresolvedReferences != 0; ++importIndex)
    {
        unsigned int existingFileIndex;
        for (existingFileIndex = 0; existingFileIndex < m_files.size() && unresolvedReferences != 0; ++existingFileIndex)
        {
            if (std::strcmp(file->importAssetData[importIndex].name, m_files[existingFileIndex]->header.name) == 0)
                unresolvedReferences = file->resolveReferences(m_files[existingFileIndex]);
        }
    }

    std::vector<fuiFile*>::reverse_iterator it = m_files.rbegin();
    while (it != m_files.rend() && unresolvedReferences != 0)
    {
        unresolvedReferences = file->resolveReferences(*it);
        ++it;
    }

    if (unresolvedReferences != 0)
        file->dumpUnresolvedReferences();

    m_files.push_back(file);

    if (createRoot)
    {
        file->stage = new fuiRenderNodeStage(file);
        file->stage->m_node = new FJ_FuiNodeStage(file->stage);
        file->rootNode = new fuiRenderNodeTimeline(file->stage, file->stage, file->timelineData, 0, 0, NULL, 0, false, file);
        FJ_FuiNode rootNode(file->rootNode);
        file->rootNode->m_node = &rootNode;
        std::strncpy(file->rootNode->m_name, "root", 64);
        file->rootNode->gather(false);
        file->stage->addChild(file->rootNode);
        file->rootNode->handleAddedToStage();
    }

    LeaveCriticalSection(&m_mutex);
    return (int)file;
}

void fui::unloadScene(fuiFile* file)
{
    if (file == NULL)
        return;

    EnterCriticalSection(&m_mutex);
    delete file;

    std::vector<fuiFile*>::iterator it = m_files.begin();
    while (it != m_files.end() && *it != file)
        ++it;

    if (it != m_files.end())
    {
        int erasedIndex = static_cast<int>(it - m_files.begin());
        m_files.erase(it);
        for (int i = 0; i < 64; ++i)
        {
            if (m_permanentFileIndices[i] > erasedIndex)
                --m_permanentFileIndices[i];
            else if (m_permanentFileIndices[i] == erasedIndex)
                m_permanentFileIndices[i] = -1;
        }
    }

    LeaveCriticalSection(&m_mutex);
}

void fui::setScreenSize(float width, float height)
{
    if (width == 1280.0f && height == 480.0f)
        width = 640.0f;
    m_screenWidth = width;
    m_screenHeight = height;
}

void fui::setResolution(int resolution)
{
    m_resolution = resolution;
    if (resolution == 0)
        setScreenSize(1280.0f, 720.0f);
    else
        setScreenSize(1920.0f, 1080.0f);
}

unsigned int fui::getResolution()
{
    return m_resolution;
}

void fui::setOrigin(float x, float y)
{
    m_originX = x;
    m_originY = y;
}

void fui::tick()
{
    m_textureManager->tick();
    ++m_frameCount;
    uint64_t currentTime = System::processTimeInMilliSecs();
    if (currentTime - m_lastTime < 34)
    {
        m_enterFrameDispatch = false;
        return;
    }
    m_enterFrameDispatch = true;
    if (currentTime - (m_lastTime + 33) > 100)
        m_lastTime = currentTime;
    else
        m_lastTime += 33;
}

void fui::preRender(fuiFile* file)
{
    file->stage->preRender();
    for (unsigned int i = 0; i < m_fonts.size(); ++i)
        m_fonts[i]->preRender();
}

void fui::render(fuiFile* file, float x, float y, float width, float height, float scaleX, float scaleY)
{
    float renderMatrix[16];
    std::memset(renderMatrix, 0, sizeof(renderMatrix));
    renderMatrix[0] = 1.0f;
    renderMatrix[5] = 1.0f;
    renderMatrix[10] = 1.0f;
    renderMatrix[15] = 1.0f;

    gRenderer->TEMP_PatchStateFromIggy();
    gRenderer->StateSetAlphaTestEnable(false);
    gRenderer->StateSetBlendEnable(true);
    gRenderer->StateSetBlendFunc(1, 5);
    gRenderer->StateSetDepthTestEnable(false);
    gRenderer->StateSetFaceCull(false);
    gRenderer->StateSetDepthMask(false);
    gRenderer->MatrixMode(1);
    gRenderer->MatrixSetIdentity();
    gRenderer->MatrixOrthogonal(0.0f, m_screenWidth, m_screenHeight, 0.0f, 1000.0f, 3000.0f);
    gRenderer->MatrixMode(0);
    gRenderer->MatrixSetIdentity();
    gRenderer->MatrixTranslate(m_originX - x, m_originY - y, -2000.0f);
    gRenderer->MatrixScale(scaleX, scaleY, 1.0f);
    gRenderer->StateSetScissorRect(static_cast<unsigned int>(m_originX), static_cast<unsigned int>(m_originY),
                                   static_cast<unsigned int>(m_originX + width), static_cast<unsigned int>(m_originY + height));
    file->stage->gather(m_enterFrameDispatch);
    preRender(file);
    file->stage->render(NULL, NULL);
}

void fui::render(fuiFile* file, float x, float y, float width, float height)
{
    render(file, x, y, width, height, 1.0f, 1.0f);
}

void fui::dispatchKeyboardEvent(fuiFile* file, bool keyUp, int keyCode)
{
    FJ_FuiNodeStage* stage = static_cast<FJ_FuiNodeStage*>(file->stage->m_node);
    FJ_FuiNode* target = stage->getFocus();
    if (target == NULL)
        target = stage;
    target->dispatchEvent(new FJ_KeyboardEvent(keyUp, keyCode, true, false));
}

void fui::dispatchMouseMoveEvent(fuiFile* file, float x, float y)
{
    FJ_FuiNodeStage* stage = static_cast<FJ_FuiNodeStage*>(file->stage->m_node);
    FJ_FuiNode* target = stage->getFocus();
    if (target == NULL)
        target = stage;
    target->dispatchEvent(new FJ_MouseMoveEvent(x, y, true, false));
}

void fui::dispatchEnterFrameEvent()
{
    if (!m_enterFrameDispatch)
        return;
    FJ_Event event(eFJEventType_EnterFrame, false, false);
    unsigned int i = 0;
    while (i < m_directEventListeners.size())
    {
        if (m_directEventListeners[i].m_eType == eFJEventType_EnterFrame)
        {
            m_directEventListeners[i].m_pfnHandler(m_directEventListeners[i].m_pNode, &event);
        }
        ++i;
    }
}

void fui::removeDirectEventListener(const FJ_EventListener& listener)
{
    std::vector<FJ_EventListener>::iterator it = m_directEventListeners.begin();
    while (it != m_directEventListeners.end())
    {
        if (it->m_eType == listener.m_eType && it->m_pNode == listener.m_pNode && it->m_pfnHandler == listener.m_pfnHandler)
        {
            m_directEventListeners.erase(it);
            return;
        }
        ++it;
    }
}

void fui::addDirectEventListener(const FJ_EventListener& listener)
{
    removeDirectEventListener(listener);
    m_directEventListeners.push_back(listener);
}

void fui::removeEventListenerForNode(FJ_FuiNode* node)
{
    std::vector<FJ_EventListener>::iterator it = m_directEventListeners.begin();
    while (it != m_directEventListeners.end())
    {
        if (it->m_pNode == node)
            it = m_directEventListeners.erase(it);
        else
            ++it;
    }
}

void fui::setCustomTextureFn(int (*textureFunction)(void*, const std::wstring&, unsigned int*, unsigned int*, bool*), void (*releaseFunction)(int), void* context)
{
    m_customTextureFunction = textureFunction;
    m_releaseCustomTextureFunction = releaseFunction;
    m_customTextureContext = context;
}

uint32_t fui::getCustomTexture(const std::wstring& name, unsigned int* width, unsigned int* height, bool* hasAlpha)
{
    if (m_customTextureFunction == NULL)
        return UINT32_MAX;
    return m_customTextureFunction(m_customTextureContext, name, width, height, hasAlpha);
}

void fui::releaseCustomTexture(int textureId)
{
    if (m_releaseCustomTextureFunction != NULL)
        m_releaseCustomTextureFunction(textureId);
}

unsigned char fui::getCurrentStencilMask()
{
    return m_stencilMask;
}

void fui::incrementStencilMask()
{
    ++m_stencilMask;
    if (m_stencilMask == 0)
        m_stencilMask = 1;
}

uint32_t fui::getFuiFileFromIndex(unsigned int index)
{
    return (uint32_t)m_files[m_permanentFileIndices[index]];
}

fuiRenderNode* fui::createNodeFromSymbol(const char* name, fuiRenderNode* parent)
{
    if (*name == '\0')
    {
        fuiRenderNodeTimeline* timeline = new fuiRenderNodeTimeline(parent->m_stage, NULL, NULL, 0, 0, NULL, 0, false, NULL);
        timeline->disableCtor();
        timeline->gather(false);
        return timeline;
    }

    fuiRenderNode* node = NULL;
    std::vector<fuiFile*>::reverse_iterator file = m_files.rbegin();
    while (file != m_files.rend() && node == NULL)
    {
        node = (fuiRenderNode*)(*file)->createNodeFromSymbol(name, parent, m_resolution);
        ++file;
    }

    if (node != NULL && node->m_type == eFuiObjectType_Timeline)
    {
        fuiRenderNodeTimeline* timelineNode = static_cast<fuiRenderNodeTimeline*>(node);
        fuiTimeline* timeline = static_cast<fuiTimeline*>(node->m_object);
        unsigned int i;
        for (i = 0; i < timeline->timelineActionCount; ++i)
        {
            fuiTimelineAction& action = node->m_file->timelineActionData[timeline->timelineActionIndex + i];
            if (action.actionType == 0x1e)
                timelineNode->addFrameAction(action.frameIndex, static_cast<fuiRenderNodeTimeline::eFrameActionType>(1), std::string("?"));
            else if (action.actionType == 0x1f)
                timelineNode->addFrameAction(action.frameIndex, static_cast<fuiRenderNodeTimeline::eFrameActionType>(2), std::string(action.actionValue));
            else if (action.actionType == 0x20)
                timelineNode->addFrameAction(action.frameIndex, static_cast<fuiRenderNodeTimeline::eFrameActionType>(3), std::string(action.actionValue));
            else if (action.actionType == 0x21)
                timelineNode->addFrameAction(action.frameIndex, static_cast<fuiRenderNodeTimeline::eFrameActionType>(0), std::string(action.actionValue));
        }
    }

    return node;
}

int fui::findBitmapFromSymbol(const char* name, fuiFile** file)
{
    *file = NULL;
    std::vector<fuiFile*>::reverse_iterator it;
    for (it = m_files.rbegin(); it != m_files.rend(); ++it)
    {
        fuiFile* currentFile = *it;
        if (currentFile->fileIndex != m_resolution)
            continue;
        for (unsigned int i = 0; i < currentFile->header.bitmapCount; ++i)
        {
            fuiBitmap& bitmap = currentFile->bitmaps[i];
            if (bitmap.symbolIndex != static_cast<unsigned int>(-1) &&
                std::strcmp(name, currentFile->symbolData[bitmap.symbolIndex].name) == 0)
            {
                *file = currentFile;
                return (int)&bitmap;
            }
        }
    }
    return 0;
}

void fui::installTrueTypeFont(int fontIndex, const void** data, const char* name, unsigned short missingGlyph)
{
    m_fonts.push_back(new fuiTTFFontSet(name, fontIndex, data, missingGlyph));
}
void fui::uninstallTrueTypeFont(const char* name)
{
    unsigned int i;
    for (i = 0; i < m_fonts.size(); ++i)
    {
        fuiFontSet* font = m_fonts[i];
        if (font->m_name == name)
        {
            if (!font->isBitmapFont())
            {
                delete font;
                m_fonts.erase(m_fonts.begin() + i);
            }
            return;
        }
    }
}

void fui::installBitmapFont(fuiBitmapFont* font)
{
    m_fonts.push_back(font);
}

void fui::setFontIndirection(const char* source, const char* target)
{
    unsigned int i;
    for (i = 0; i < m_fontIndirections.size(); ++i)
    {
        if (m_fontIndirections[i].source == source)
        {
            m_fontIndirections[i].target = target;
            return;
        }
    }
    fuiFontIndirection value;
    value.source = source;
    value.target = target;
    m_fontIndirections.push_back(value);
}

unsigned int fui::getFont(const char* name, bool skipIndirection)
{
    std::string fontName(name);
    unsigned int i;
    if (!skipIndirection)
    {
        for (i = 0; i < m_fontIndirections.size(); ++i)
        {
            if (m_fontIndirections[i].source == name)
            {
                fontName = m_fontIndirections[i].target;
                break;
            }
        }
    }
    for (i = 0; i < m_fonts.size(); ++i)
    {
        if (m_fonts[i]->m_name == fontName)
            return (unsigned int)m_fonts[i];
    }
    return 0;
}

unsigned int fui::getDefaultFont(bool skipIndirection)
{
    return getFont(m_fonts[0]->m_name.c_str(), skipIndirection);
}

unsigned int fui::GetCurrentFrameCount()
{
    return m_frameCount;
}
