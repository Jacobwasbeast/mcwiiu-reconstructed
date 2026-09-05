#include "game/core/ConsoleSchematicFile.hpp"
#include <cstring>
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/NbtIo.hpp"
#include "game/data/ListTag.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/core/Rotation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/EntityIO.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/BlockTransaction.hpp"
#include "game/world/chunk/LevelChunk.hpp"

ConsoleSchematicFile::PasteData::PasteData()
{
}

ConsoleSchematicFile::ConsoleSchematicFile()
    : mValid(1), mOrigin(0, 0, 0), mWidth(0), mHeight(0), mDepth(0), mIncludeAir(false), mIncludeMobs(false), mCopyArea(0), mUndoArea(0), mCopyMinX(0), mCopyMinZ(0), mCopyMaxX(0), mUndoWidth(0), mUndoHeight(0), mUndoDepth(0), mStoredCopyArea(0), mStoredUndoArea(0), mUndoReady(false), mPasteTicks(0), mPasteState(0), mHasCopy(false), mHasPaste(false), mMobs(false), mReserved(false)
{
    mPasteData.mLevel = 0;
    mPasteData.mArea = AABB::newPermanent(0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
}

ConsoleSchematicFile::~ConsoleSchematicFile()
{
    delete[] mElements.data;
    delete[] mExtraElements.data;
    delete[] mData.data;
    delete[] mExtraData.data;
    delete mCopyArea;
    delete mUndoArea;
    delete[] mUndoElements.data;
    delete[] mUndoExtraElements.data;
    delete mStoredCopyArea;
    delete mStoredUndoArea;
}

bool ConsoleSchematicFile::CanPasteBlock(unsigned short element, bool includeAir, bool includeSpecial)
{
    if (includeAir && element == 0) return true;
    if (element == 0) return false;
    if (includeSpecial) return true;
    return (element >> 4) != 8 && (element >> 4) != 9;
}

unsigned short ConsoleSchematicFile::rotateElementValue(unsigned short element, ESchematicRotation rotation)
{
    if (rotation == eSchematicRotation_0)
    {
        return element;
    }

    const BlockState* state = Block::getStateByElementId(element);
    Rotation* blockRotation = Rotation::NONE;
    if (rotation == eSchematicRotation_90) blockRotation = Rotation::CLOCKWISE_90;
    else if (rotation == eSchematicRotation_180) blockRotation = Rotation::CLOCKWISE_180;
    else if (rotation == eSchematicRotation_270) blockRotation = Rotation::COUNTERCLOCKWISE_90;
    return (unsigned short)Block::getElementId(state->rotate(blockRotation));
}

void ConsoleSchematicFile::chunkCoordToSchematicCoord(AABB* box, int x, int z, ESchematicRotation rotation, ESchematicMirror mirror, int& sx, int& sz)
{
    sx = x - (int)box->mMinX;
    sz = z - (int)box->mMinZ;
    if (mirror == eSchematicMirror_LeftRight || mirror == eSchematicMirror_Both) sx = mWidth - sx - 1;
    if (mirror == eSchematicMirror_FrontBack || mirror == eSchematicMirror_Both) sz = mDepth - sz - 1;
    if (rotation == eSchematicRotation_90) { int t = sx; sx = sz; sz = mWidth - t - 1; }
    else if (rotation == eSchematicRotation_180) { sx = mWidth - sx - 1; sz = mDepth - sz - 1; }
    else if (rotation == eSchematicRotation_270) { int t = sx; sx = mDepth - sz - 1; sz = t; }
}

void ConsoleSchematicFile::schematicCoordToChunkCoord(AABB* box, double x, double z, ESchematicRotation rotation, ESchematicMirror mirror, double& cx, double& cz)
{
    cx = x + box->mMinX;
    cz = z + box->mMinZ;
    if (mirror == eSchematicMirror_LeftRight || mirror == eSchematicMirror_Both) cx = box->mMaxX - x;
    if (mirror == eSchematicMirror_FrontBack || mirror == eSchematicMirror_Both) cz = box->mMaxZ - z;
    if (rotation == eSchematicRotation_90) { double t = cx; cx = cz; cz = box->mMaxX - t; }
    else if (rotation == eSchematicRotation_180) { cx = box->mMaxX - x; cz = box->mMaxZ - z; }
    else if (rotation == eSchematicRotation_270) { double t = cx; cx = box->mMaxZ - cz; cz = t; }
}

const Direction* ConsoleSchematicFile::HandleRotation(const Direction* direction, ESchematicMirror mirror, ESchematicRotation rotation)
{
    const Direction* result = direction;
    if (mirror == eSchematicMirror_LeftRight || mirror == eSchematicMirror_Both) { if (result == Direction::EAST) result = Direction::WEST; else if (result == Direction::WEST) result = Direction::EAST; }
    if (mirror == eSchematicMirror_FrontBack || mirror == eSchematicMirror_Both) { if (result == Direction::NORTH) result = Direction::SOUTH; else if (result == Direction::SOUTH) result = Direction::NORTH; }
    for (int i = 0; i < (int)rotation; ++i) result = result->getClockWise();
    return result;
}

int ConsoleSchematicFile::calcBlockIdsSizeLegacy(int x, int y, int z) { return x * y * z; }
int ConsoleSchematicFile::calcHalfByteDataSizeLegacy(int x, int y, int z) { return x * ((y + 1) / 2) * z; }
int ConsoleSchematicFile::calcFullDataSizeLegacy(int x, int y, int z) { return calcBlockIdsSizeLegacy(x, y, z) + calcHalfByteDataSizeLegacy(x, y, z); }
int ConsoleSchematicFile::calculateFullElementSize(int x, int y, int z) { return x * y * z; }

std::wstring ConsoleSchematicFile::formatFilenameWithExt(const std::wstring& filename)
{
    if (filename.size() >= 4 && filename.compare(filename.size() - 4, 4, L".sch") == 0) return filename;
    return filename + L".sch";
}

void ConsoleSchematicFile::save(DataOutputStream* output)
{
    if (output == 0)
    {
        return;
    }

    output->writeInt(3);
    output->writeByte(3);
    output->writeInt(mWidth);
    output->writeInt(mHeight);
    output->writeInt(mDepth);

    unsigned int compressedSize = mElements.length * sizeof(unsigned short);
    arrayWithLength<unsigned char> compressed(compressedSize, true);
    Compression::getCompression()->CompressLZXRLE(compressed.data, &compressedSize, mElements.data, mElements.length * sizeof(unsigned short), 0);
    compressed.length = compressedSize;
    output->writeInt(compressedSize);
    output->write(compressed);
    delete[] compressed.data;

    compressedSize = mExtraElements.length * sizeof(unsigned short);
    arrayWithLength<unsigned char> extraCompressed(compressedSize, true);
    Compression::getCompression()->CompressLZXRLE(extraCompressed.data, &compressedSize, mExtraElements.data, mExtraElements.length * sizeof(unsigned short), 0);
    extraCompressed.length = compressedSize;
    output->writeInt(compressedSize);
    output->write(extraCompressed);
    delete[] extraCompressed.data;
    save_tags(output);
}

void ConsoleSchematicFile::save_tags(DataOutputStream* output)
{
    CompoundTag* root = new CompoundTag();
    ListTag* blockEntities = new ListTag();
    for (std::vector<std::pair<BlockPos*, CompoundTag*> >::iterator it = mBlockEntities.begin(); it != mBlockEntities.end(); ++it)
    {
        blockEntities->add(it->second->copy());
    }
    root->put(L"TileEntities", blockEntities);

    ListTag* entities = new ListTag();
    for (std::vector<std::pair<Vec3*, CompoundTag*> >::iterator it = mEntities.begin(); it != mEntities.end(); ++it)
    {
        entities->add(it->second->copy());
    }
    root->put(L"Entities", entities);
    NbtIo::write(root, output);
    delete root;
}

void ConsoleSchematicFile::load(DataInputStream* input)
{
    if (input == 0)
    {
        return;
    }

    int version = input->readInt();
    unsigned char compression = input->readByte();
    mWidth = input->readInt();
    mHeight = input->readInt();
    mDepth = input->readInt();
    unsigned int elementCount = calculateFullElementSize(mWidth, mHeight, mDepth);
    mElements.resize(elementCount);
    mExtraElements.resize(elementCount);

    unsigned int compressedSize = input->readInt();
    arrayWithLength<unsigned char> compressed(compressedSize, false);
    input->readFully(compressed);
    unsigned int outputSize = elementCount * sizeof(unsigned short);
    if (compression == Compression::eCompressionTypes_RLE)
        Compression::getCompression()->DecompressRLE(mElements.data, &outputSize, compressed.data, compressed.length);
    else
        Compression::getCompression()->DecompressLZXRLE(mElements.data, &outputSize, compressed.data, compressed.length);
    delete[] compressed.data;

    compressedSize = input->readInt();
    arrayWithLength<unsigned char> extraCompressed(compressedSize, false);
    input->readFully(extraCompressed);
    outputSize = elementCount * sizeof(unsigned short);
    if (compression == Compression::eCompressionTypes_RLE)
        Compression::getCompression()->DecompressRLE(mExtraElements.data, &outputSize, extraCompressed.data, extraCompressed.length);
    else
        Compression::getCompression()->DecompressLZXRLE(mExtraElements.data, &outputSize, extraCompressed.data, extraCompressed.length);
    delete[] extraCompressed.data;

    CompoundTag* tags = NbtIo::read(input);
    mBlockEntities.clear();
    mEntities.clear();
    if (tags != 0)
    {
        ListTag* tileTags = tags->getList(L"TileEntities", 10);
        for (int i = 0; i < tileTags->size(); ++i)
        {
            CompoundTag* tag = tileTags->getCompound(i)->copy();
            mBlockEntities.push_back(std::make_pair(new BlockPos(tag->getInt(L"x"), tag->getInt(L"y"), tag->getInt(L"z")), tag));
        }
        ListTag* entityTags = tags->getList(L"Entities", 10);
        for (int i = 0; i < entityTags->size(); ++i)
        {
            CompoundTag* tag = entityTags->getCompound(i)->copy();
            ListTag* position = tag->getList(L"Pos", 6);
            mEntities.push_back(std::make_pair(Vec3::newPermanent(position->getDouble(0), position->getDouble(1), position->getDouble(2)), tag));
        }
        delete tags;
    }
    mValid = version != 0;
}

void ConsoleSchematicFile::setElements(arrayWithLength<unsigned short>& out, arrayWithLength<unsigned short> in, ESchematicRotation rotation, ESchematicMirror mirror, int x0, int y0, int z0, int x1, int y1, int z1, int& count, bool includeAir, bool includeSpecial)
{
    for (int x = x0; x < x1; ++x)
    {
        for (int z = z0; z < z1; ++z)
        {
            for (int y = y0; y < y1; ++y)
            {
                int sourceIndex = mirror == eSchematicMirror_FrontBack ? count-- : count++;
                unsigned short element = in.data[sourceIndex];
                unsigned int destinationIndex = (unsigned int)((x << 11) | (z << 7) | y);
                if (destinationIndex < out.length && CanPasteBlock(element, includeAir, includeSpecial))
                {
                    out.data[destinationIndex] = rotateElementValue(element, rotation);
                }
            }
        }
    }
}

void ConsoleSchematicFile::pasteIntoArray(arrayWithLength<unsigned char>& destination, arrayWithLength<unsigned char> source, arrayWithLength<unsigned short> elements, int elementIndex, int sourceIndex, int destinationIndex, bool swapNibbles, bool includeAir, bool includeSpecial)
{
    bool low = CanPasteBlock(elements.data[elementIndex], includeAir, includeSpecial);
    bool high = CanPasteBlock(elements.data[elementIndex + 1], includeAir, includeSpecial);
    unsigned char value = source.data[sourceIndex];
    if (swapNibbles)
    {
        value = (unsigned char)((value << 4) | (value >> 4));
    }
    if (low && high)
    {
        destination.data[destinationIndex] = value;
    }
    else
    {
        if (low) destination.data[destinationIndex] = (unsigned char)((destination.data[destinationIndex] & 0xf0) | (value & 0x0f));
        if (high) destination.data[destinationIndex] = (unsigned char)((destination.data[destinationIndex] & 0x0f) | (value & 0xf0));
    }
}

void ConsoleSchematicFile::setData(arrayWithLength<unsigned char>& out, arrayWithLength<unsigned char> in, ESchematicMirror mirror, int x0, int y0, int z0, int x1, int y1, int z1, int& count, bool includeAir, bool includeSpecial, arrayWithLength<unsigned short> elements, int& index)
{
    bool swapNibbles = (y0 & 1) != 0;
    for (int x = x0; x < x1; ++x)
    {
        for (int z = z0; z < z1; ++z)
        {
            for (int y = y0; y < y1; y += 2)
            {
                if (count >= 0 && count < (int)out.length && index >= 0 && index < (int)in.length)
                {
                    pasteIntoArray(out, in, elements, index * 2, index, count, swapNibbles, includeAir, includeSpecial);
                }
                ++count;
                if (mirror == eSchematicMirror_FrontBack) --index;
                else ++index;
            }
        }
    }
}

void ConsoleSchematicFile::getElements(LevelChunk* chunk, arrayWithLength<unsigned short>* elements, arrayWithLength<unsigned short>* extra, int x, int y, int z, int width, int height, int depth, int& count)
{
    count = 0; if (chunk == 0 || elements == 0) return; for (int dx=0; dx<width; ++dx) for (int dy=0; dy<height; ++dy) for (int dz=0; dz<depth; ++dz) { unsigned int i=(unsigned int)((dx*height+dy)*depth+dz); if(i<elements->length) elements->data[i]=chunk->getElement(x+dx,y+dy,z+dz); ++count; }
}

void ConsoleSchematicFile::getData(arrayWithLength<unsigned char> in, arrayWithLength<unsigned char>* out, int& count, int x, int y, int z, int width, int height, int depth)
{
    count=0; if(out==0) return; while(count<(int)in.length && count<(int)out->length) { out->data[count]=in.data[count]; ++count; }
}

void ConsoleSchematicFile::fillArraysFromLevel(Level* level, int x, int y, int z, int width, int height, int depth, arrayWithLength<unsigned short>* elements, arrayWithLength<unsigned short>* extra, arrayWithLength<unsigned char>* data, arrayWithLength<unsigned char>* extraData)
{
    if(level==0 || elements==0) return; for(int dx=0;dx<width;++dx) for(int dy=0;dy<height;++dy) for(int dz=0;dz<depth;++dz) { unsigned int i=(unsigned int)((dx*height+dy)*depth+dz); if(i<elements->length) elements->data[i]=level->getElement(x+dx,y+dy,z+dz); }
}

void ConsoleSchematicFile::applyBlocksAndData(LevelChunk* chunk, PasteData* data)
{
    if (chunk == 0 || data == 0)
    {
        return;
    }

    int chunkMinX = chunk->x << 4;
    int chunkMinZ = chunk->z << 4;
    int x0 = (int)data->mArea->mMinX > chunkMinX ? (int)data->mArea->mMinX : chunkMinX;
    int z0 = (int)data->mArea->mMinZ > chunkMinZ ? (int)data->mArea->mMinZ : chunkMinZ;
    int x1 = (int)data->mArea->mMaxX < chunkMinX + 16 ? (int)data->mArea->mMaxX : chunkMinX + 16;
    int z1 = (int)data->mArea->mMaxZ < chunkMinZ + 16 ? (int)data->mArea->mMaxZ : chunkMinZ + 16;
    writeSchematicIntoArrays(chunkMinX, 0, chunkMinZ, x0, (int)data->mArea->mMinY, z0, x1, (int)data->mArea->mMaxY, z1, data, data->mElements, data->mExtraElements, data->mData, data->mExtraData);
    chunk->setAllElements(data->mElements, data->mExtraElements);
}

void ConsoleSchematicFile::applyBlocksAndData(LevelChunk* chunk, AABB* box, ESchematicRotation rotation, ESchematicMirror mirror)
{
    PasteData data;
    data.mArea = box;
    data.mRotation = rotation;
    data.mMirror = mirror;
    data.mElements = mElements;
    data.mExtraElements = mExtraElements;
    data.mData = mData;
    data.mExtraData = mExtraData;
    data.mBlockEntities = mBlockEntities;
    data.mEntities = mEntities;
    data.mNamedPositions = mNamedPositions;
    data.mIncludeAir = true;
    data.mIncludeSpecial = false;
    applyBlocksAndData(chunk, &data);
}
void ConsoleSchematicFile::applyBlockEntities(LevelChunk* chunk, AABB* chunkArea, PasteData* paste, Level* level)
{
    for (std::vector<std::pair<BlockPos*, CompoundTag*> >::iterator it = paste->mBlockEntities.begin(); it != paste->mBlockEntities.end(); ++it)
    {
        double x;
        double z;
        schematicCoordToChunkCoord(paste->mArea, it->first->x, it->first->z, paste->mRotation, paste->mMirror, x, z);
        BlockPos pos(x, paste->mArea->mMinY + it->first->y, z);
        if (chunkArea->contains(pos))
        {
            CompoundTag* tag = it->second->copy();
            tag->putInt(L"x", pos.x);
            tag->putInt(L"y", pos.y);
            tag->putInt(L"z", pos.z);
            boost::shared_ptr<BlockEntity> blockEntity = BlockEntity::loadStatic(level, tag);
            if (blockEntity) chunk->addBlockEntity(blockEntity);
            delete tag;
        }
    }

    for (std::vector<std::pair<Vec3*, CompoundTag*> >::iterator it = paste->mEntities.begin(); it != paste->mEntities.end(); ++it)
    {
        double x;
        double z;
        schematicCoordToChunkCoord(paste->mArea, it->first->x, it->first->z, paste->mRotation, paste->mMirror, x, z);
        double y = paste->mArea->mMinY + it->first->y;
        if (chunkArea->contains(x, y, z))
        {
            boost::shared_ptr<Entity> entity = EntityIO::newEntity(it->second, level);
            if (entity)
            {
                entity->setPos(x, y, z);
                level->addEntity(entity);
            }
        }
    }
}

void ConsoleSchematicFile::applyBlockEntities(LevelChunk* chunk, AABB* chunkArea, AABB* area, ESchematicRotation rotation, ESchematicMirror mirror, Level* level)
{
    PasteData paste;
    paste.mArea = area;
    paste.mRotation = rotation;
    paste.mMirror = mirror;
    paste.mBlockEntities = mBlockEntities;
    paste.mEntities = mEntities;
    applyBlockEntities(chunk, chunkArea, &paste, level);
}

int ConsoleSchematicFile::applyLighting(LevelChunk* chunk, AABB*, AABB*, ESchematicRotation)
{
    if (chunk == 0)
    {
        return 0;
    }
    chunk->recalcHeightmapOnly();
    chunk->recalcBlockLights();
    chunk->setUnsaved(true);
    return 1;
}

void ConsoleSchematicFile::offsetPainting(Level*, boost::shared_ptr<Painting>, ESchematicRotation, ESchematicMirror)
{
}
void ConsoleSchematicFile::_applySchematicData(Level* level, AABB* area, ESchematicRotation rotation, arrayWithLength<unsigned short> elements, arrayWithLength<unsigned short> extraElements, arrayWithLength<unsigned char> data, arrayWithLength<unsigned char> extraData, std::vector<std::pair<BlockPos*, CompoundTag*> >& blockEntities, std::vector<std::pair<Vec3*, CompoundTag*> >& entities, std::vector<DevSaveDataManager::NamedPositionOrVolume>& namedPositions, int width, int height, int depth, bool includeAir, bool includeSpecial, bool rotate)
{
    mPasteData.mLevel = level;
    mPasteData.mArea->set(area);
    mPasteData.mRotation = rotate ? rotation : eSchematicRotation_0;
    mPasteData.mMirror = (ESchematicMirror)mPasteState;
    mPasteData.mMinChunkX = (int)area->mMinX >> 4;
    mPasteData.mMaxChunkX = (int)area->mMaxX >> 4;
    mPasteData.mMinChunkZ = (int)area->mMinZ >> 4;
    mPasteData.mMaxChunkZ = (int)area->mMaxZ >> 4;
    mPasteData.mCurrentChunkX = mPasteData.mMinChunkX;
    mPasteData.mCurrentChunkZ = mPasteData.mMinChunkZ;
    mPasteData.mWidth = width;
    mPasteData.mHeight = height;
    mPasteData.mDepth = depth;
    mPasteData.mElements = elements;
    mPasteData.mExtraElements = extraElements;
    mPasteData.mData = data;
    mPasteData.mExtraData = extraData;
    mPasteData.mBlockEntities = blockEntities;
    mPasteData.mEntities = entities;
    mPasteData.mNamedPositions = namedPositions;
    mPasteData.mIncludeAir = includeAir;
    mPasteData.mIncludeSpecial = includeSpecial;
    mHasCopy = true;
}
bool ConsoleSchematicFile::undoPrevious(Level* level)
{
    if (!mIncludeAir || mCopyArea == 0)
    {
        return false;
    }
    _applySchematicData(level, mCopyArea, eSchematicRotation_0, mUndoElements, mUndoExtraElements, mData, mExtraData, mUndoBlockEntities, mUndoEntities, mUndoNamedPositions, mUndoWidth, mUndoHeight, mUndoDepth, true, true, false);
    mIncludeAir = false;
    return true;
}
int ConsoleSchematicFile::writeSchematicIntoArrays(int chunkX, int chunkY, int chunkZ, int x0, int y0, int z0, int x1, int y1, int z1, PasteData* paste, arrayWithLength<unsigned short> elements, arrayWithLength<unsigned short> extraElements, arrayWithLength<unsigned char> data, arrayWithLength<unsigned char> extraData)
{
    int volume = (x1 - x0) * (y1 - y0) * (z1 - z0);
    int halfHeight = (paste->mHeight + 1) / 2;
    for (int x = x0; x < x1; ++x)
    {
        for (int z = z0; z < z1; ++z)
        {
            int schematicX;
            int schematicZ;
            chunkCoordToSchematicCoord(paste->mArea, x, z, paste->mRotation, paste->mMirror, schematicX, schematicZ);
            int elementIndex = schematicX * paste->mDepth * paste->mHeight + schematicZ * paste->mHeight + (y0 - chunkY);
            int localX = x - chunkX;
            int localZ = z - chunkZ;
            setElements(elements, paste->mElements, paste->mRotation, paste->mMirror, localX, y0, localZ, localX + 1, y1, localZ + 1, elementIndex, paste->mIncludeAir, paste->mIncludeSpecial);
            int extraIndex = schematicX * paste->mDepth * paste->mHeight + schematicZ * paste->mHeight + (y0 - chunkY);
            setElements(extraElements, paste->mExtraElements, paste->mRotation, paste->mMirror, localX, y0, localZ, localX + 1, y1, localZ + 1, extraIndex, paste->mIncludeAir, paste->mIncludeSpecial);
            int destinationDataIndex = schematicX * paste->mDepth * halfHeight + schematicZ * halfHeight + ((y0 - chunkY) >> 1);
            int sourceDataIndex = destinationDataIndex;
            setData(data, paste->mData, paste->mMirror, localX, y0, localZ, localX + 1, y1, localZ + 1, destinationDataIndex, paste->mIncludeAir, paste->mIncludeSpecial, paste->mElements, sourceDataIndex);
            destinationDataIndex = schematicX * paste->mDepth * halfHeight + schematicZ * halfHeight + ((y0 - chunkY) >> 1);
            sourceDataIndex = destinationDataIndex;
            setData(extraData, paste->mExtraData, paste->mMirror, localX, y0, localZ, localX + 1, y1, localZ + 1, destinationDataIndex, paste->mIncludeAir, paste->mIncludeSpecial, paste->mExtraElements, sourceDataIndex);
        }
    }
    return volume >> 31;
}
void ConsoleSchematicFile::tick()
{
    if (!mHasCopy || mPasteData.mLevel == 0)
    {
        return;
    }
    if (mPasteData.mCurrentChunkX > mPasteData.mMaxChunkX || mPasteData.mCurrentChunkZ > mPasteData.mMaxChunkZ)
    {
        mHasCopy = false;
        return;
    }

    LevelChunk* chunk = mPasteData.mLevel->getChunk(mPasteData.mCurrentChunkX, mPasteData.mCurrentChunkZ);
    if (chunk != 0 && !chunk->isUnloaded())
    {
        double minX = mPasteData.mCurrentChunkX * 16.0;
        double minZ = mPasteData.mCurrentChunkZ * 16.0;
        AABB* chunkArea = AABB::newTemp(minX, mPasteData.mArea->mMinY, minZ, minX + 16.0, mPasteData.mArea->mMaxY, minZ + 16.0);
        applyBlocksAndData(chunk, &mPasteData);
        applyLighting(chunk, chunkArea, mPasteData.mArea, mPasteData.mRotation);
        applyBlockEntities(chunk, chunkArea, &mPasteData, mPasteData.mLevel);
    }

    ++mPasteData.mCurrentChunkZ;
    if (mPasteData.mCurrentChunkZ > mPasteData.mMaxChunkZ)
    {
        ++mPasteData.mCurrentChunkX;
        mPasteData.mCurrentChunkZ = mPasteData.mMinChunkZ;
    }
}

std::vector<boost::shared_ptr<BlockEntity> >* ConsoleSchematicFile::getBlockEntitiesInRegion(LevelChunk* chunk, int x0, int y0, int z0, int x1, int y1, int z1)
{
    std::vector<boost::shared_ptr<BlockEntity> >* result = new std::vector<boost::shared_ptr<BlockEntity> >();
    if (chunk == 0)
    {
        return result;
    }
    LevelChunk::BlockEntityMap* blockEntities = chunk->getBlockEntities();
    for (LevelChunk::BlockEntityMap::iterator it = blockEntities->begin(); it != blockEntities->end(); ++it)
    {
        BlockPos pos = it->second->getBlockPos();
        if (pos.x >= x0 && pos.y >= y0 && pos.z >= z0 && pos.x < x1 && pos.y < y1 && pos.z < z1)
        {
            result->push_back(it->second);
        }
    }
    return result;
}

void ConsoleSchematicFile::getBlockEntitiesInRegion(Level* level, int x0, int y0, int z0, int x1, int y1, int z1, std::vector<std::pair<BlockPos*, CompoundTag*> >& result)
{
    for (int chunkX = x0 >> 4; chunkX <= x1 >> 4; ++chunkX)
    {
        for (int chunkZ = z0 >> 4; chunkZ <= z1 >> 4; ++chunkZ)
        {
            std::vector<boost::shared_ptr<BlockEntity> >* blockEntities = getBlockEntitiesInRegion(level->getChunk(chunkX, chunkZ), x0, y0, z0, x1, y1, z1);
            for (std::vector<boost::shared_ptr<BlockEntity> >::iterator it = blockEntities->begin(); it != blockEntities->end(); ++it)
            {
                CompoundTag* tag = new CompoundTag();
                (*it)->save(tag);
                BlockPos relative = (*it)->getBlockPos().offset(-x0, -y0, -z0);
                tag->putInt(L"x", relative.x);
                tag->putInt(L"y", relative.y);
                tag->putInt(L"z", relative.z);
                result.push_back(std::make_pair(new BlockPos(relative), tag));
            }
            delete blockEntities;
        }
    }
}

void ConsoleSchematicFile::getEntitiesInRegion(Level* level, int x0, int y0, int z0, int x1, int y1, int z1, std::vector<std::pair<Vec3*, CompoundTag*> >& result)
{
    AABB bounds((double)x0, (double)y0, (double)z0, (double)x1, (double)y1, (double)z1);
    for (int chunkX = x0 >> 4; chunkX <= x1 >> 4; ++chunkX)
    {
        for (int chunkZ = z0 >> 4; chunkZ <= z1 >> 4; ++chunkZ)
        {
            std::vector<boost::shared_ptr<Entity> > entities;
            level->getChunk(chunkX, chunkZ)->getEntities(boost::shared_ptr<Entity>(), &bounds, entities, 0);
            for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities.begin(); it != entities.end(); ++it)
            {
                CompoundTag* tag = new CompoundTag();
                if ((*it)->save(tag))
                {
                    result.push_back(std::make_pair(Vec3::newPermanent((*it)->mX - x0, (*it)->mY - y0, (*it)->mZ - z0), tag));
                }
                else
                {
                    delete tag;
                }
            }
        }
    }
}
ListTag* ConsoleSchematicFile::getBlockEntitiesInRegionAsTag(Level* level, int x0, int y0, int z0, int x1, int y1, int z1)
{
    ListTag* result = new ListTag();
    std::vector<std::pair<BlockPos*, CompoundTag*> > values;
    getBlockEntitiesInRegion(level, x0, y0, z0, x1, y1, z1, values);
    for (std::vector<std::pair<BlockPos*, CompoundTag*> >::iterator it = values.begin(); it != values.end(); ++it)
    {
        result->add(it->second);
    }
    return result;
}

ListTag* ConsoleSchematicFile::getEntitiesInRegionAsTag(Level* level, int x0, int y0, int z0, int x1, int y1, int z1)
{
    ListTag* result = new ListTag();
    std::vector<std::pair<Vec3*, CompoundTag*> > values;
    getEntitiesInRegion(level, x0, y0, z0, x1, y1, z1, values);
    for (std::vector<std::pair<Vec3*, CompoundTag*> >::iterator it = values.begin(); it != values.end(); ++it)
    {
        result->add(it->second);
    }
    return result;
}
bool ConsoleSchematicFile::generateSchematicFile(DataOutputStream* output, Level* level, int x0, int y0, int z0, int x1, int y1, int z1, Compression::ECompressionTypes compression)
{
    int width = x1 - x0;
    int height = y1 - y0;
    int depth = z1 - z0;
    if (width <= 0 || height <= 0 || depth <= 0)
    {
        return false;
    }

    output->writeInt(3);
    output->writeByte((unsigned char)compression);
    output->writeInt(width);
    output->writeInt(height);
    output->writeInt(depth);

    unsigned int elementCount = calculateFullElementSize(width, height, depth);
    arrayWithLength<unsigned short> elements(elementCount, true);
    arrayWithLength<unsigned short> extraElements(elementCount, true);
    fillArraysFromLevel(level, x0, y0, z0, width, height, depth, &elements, &extraElements, 0, 0);

    unsigned int byteCount = elementCount * sizeof(unsigned short);
    arrayWithLength<unsigned char> compressed(byteCount, true);
    unsigned int compressedSize = byteCount;
    if (compression == Compression::eCompressionTypes_RLE)
        Compression::getCompression()->CompressRLE(compressed.data, &compressedSize, elements.data, byteCount);
    else if (compression == Compression::eCompressionTypes_LZXRLE)
        Compression::getCompression()->CompressLZXRLE(compressed.data, &compressedSize, elements.data, byteCount, 0);
    else
        std::memcpy(compressed.data, elements.data, byteCount);
    compressed.length = compressedSize;
    output->writeInt(compressedSize);
    output->write(compressed);

    compressedSize = byteCount;
    if (compression == Compression::eCompressionTypes_RLE)
        Compression::getCompression()->CompressRLE(compressed.data, &compressedSize, extraElements.data, byteCount);
    else if (compression == Compression::eCompressionTypes_LZXRLE)
        Compression::getCompression()->CompressLZXRLE(compressed.data, &compressedSize, extraElements.data, byteCount, 0);
    else
        std::memcpy(compressed.data, extraElements.data, byteCount);
    compressed.length = compressedSize;
    output->writeInt(compressedSize);
    output->write(compressed);

    delete[] compressed.data;
    delete[] elements.data;
    delete[] extraElements.data;

    CompoundTag tags;
    tags.put(L"TileEntities", getBlockEntitiesInRegionAsTag(level, x0, y0, z0, x1, y1, z1));
    tags.put(L"Entities", getEntitiesInRegionAsTag(level, x0, y0, z0, x1, y1, z1));
    NbtIo::write(&tags, output);
    return true;
}
void ConsoleSchematicFile::applySchematicFile(BlockTransaction* transaction)
{
    int xOffset = (mWidth - transaction->mSize.x) / 2;
    int yOffset = mHeight - transaction->mSize.y;
    int zOffset = (mDepth - transaction->mSize.z) / 2;
    for (int x = 0; x < transaction->mSize.x; ++x)
    {
        for (int z = 0; z < transaction->mSize.z; ++z)
        {
            for (int y = 0; y < transaction->mSize.y; ++y)
            {
                int sx = x + xOffset;
                int sy = y + yOffset;
                int sz = z + zOffset;
                unsigned short element = 0;
                if (sx >= 0 && sx < mWidth && sy >= 0 && sy < mHeight && sz >= 0 && sz < mDepth)
                {
                    element = mElements.data[(sx * mDepth * mHeight) + (sz * mHeight) + sy];
                }
                transaction->setElement(Vec3i(x, y, z), element);
            }
        }
    }
}
