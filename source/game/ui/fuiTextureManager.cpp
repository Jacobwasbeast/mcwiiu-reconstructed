#include "game/ui/fuiTextureManager.hpp"

#include "game/client/render/Renderer.hpp"
#include "game/core/System.hpp"
#include "game/ui/fuiBitmap.hpp"

fuiTextureManager::fuiTextureManager()
    : textureCount(0), textures(0), managedTextureCount(0), managedTextureMemory(0),
      peakTextureCount(0), peakTextureMemory(0), tickCount(0), initialised(false)
{
    InitializeCriticalSection(&mutex);
}

void fuiTextureManager::initialise()
{
    textureCount = (unsigned int)gRenderer->GetMaxTextures();
    textures = new TextureEntry[textureCount]();
    initialised = true;
}

void fuiTextureManager::removeManagedTexture(fuiFile*, fuiBitmap* bitmap)
{
    if (!initialised)
        initialise();
    if (bitmap->textureId != -1) {
        TextureEntry& entry = textures[bitmap->textureId];
        if (entry.bitmap != 0) {
            EnterCriticalSection(&mutex);
            --managedTextureCount;
            managedTextureMemory -= (int)(bitmap->height * bitmap->width * 4);
            entry.bitmap = 0;
            entry.file = 0;
            entry.lastUsed = 0;
            bitmap->textureId = 0;
            LeaveCriticalSection(&mutex);
        }
    }
}

void fuiTextureManager::manageTexture(fuiFile* file, fuiBitmap* bitmap)
{
    if (!initialised)
        initialise();
    if (bitmap->textureId != -1) {
        TextureEntry& entry = textures[bitmap->textureId];
        if (entry.bitmap == 0) {
            EnterCriticalSection(&mutex);
            unsigned int count = (unsigned int)(managedTextureCount + 1);
            entry.file = file;
            entry.bitmap = bitmap;
            managedTextureCount = (int)count;
            if (peakTextureCount < count)
                peakTextureCount = count;
            unsigned int memory = (unsigned int)(managedTextureMemory + bitmap->height * bitmap->width * 4);
            managedTextureMemory = (int)memory;
            if (peakTextureMemory < memory)
                peakTextureMemory = memory;
            LeaveCriticalSection(&mutex);
        }
        unsigned int now = System::processTimeInMilliSecs();
        entry.lastUsed = now;
    }
}

void fuiTextureManager::tick()
{
    if (!initialised)
        initialise();
    ++tickCount;
    if ((tickCount % 20) == 0) {
        unsigned int now = System::processTimeInMilliSecs();
        EnterCriticalSection(&mutex);
        int count = (int)textureCount;
        int index = 0;
        while (index < count) {
            TextureEntry& entry = textures[index];
            if (entry.lastUsed != 0) {
                if ((uint64_t)now > entry.lastUsed && (uint64_t)now - entry.lastUsed > 1000) {
                    gRenderer->TextureFree(entry.bitmap->textureId);
                    removeManagedTexture(entry.file, entry.bitmap);
                    count = (int)textureCount;
                }
            }
            ++index;
        }
        LeaveCriticalSection(&mutex);
    }
}
