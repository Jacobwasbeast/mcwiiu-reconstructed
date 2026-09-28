#include "game/core/ConsoleSchematicFile.hpp"
#include "game/core/EntityType.hpp"
#include <cmath>
#include <cstring>
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/NbtIo.hpp"
#include "game/client/render/GameRenderer.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/core/StringConversion.hpp"
#include "game/core/System.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/world/chunk/PlayerChunkMap.hpp"
#include "game/data/DataFixTypes.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/core/Painting.hpp"
#include "game/data/ListTag.hpp"
#include "game/data/DoubleTag.hpp"
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
#include "game/world/block/BlockState.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/SignBlockEntity.hpp"
#include "game/world/block/TypedBoxed.hpp"

static const int SCHEMATIC_WORLD_HEIGHT = 256;

ConsoleSchematicFile::PasteData::PasteData()
    : mLevel(NULL), mArea(NULL), mRotation(eSchematicRotation_0), mMirror(eSchematicMirror_None),
      mMinChunkX(0), mMaxChunkX(0), mCurrentChunkX(0), mMinChunkZ(0), mMaxChunkZ(0), mCurrentChunkZ(0),
      mWidth(0), mHeight(0), mDepth(0),
      mElements(), mExtraElements(), mData(), mExtraData(), mBlockEntities(), mEntities(),
      mNamedPositions(), mUpdatedNames(), mIncludeAir(false), mIncludeSpecial(false)
{
}

ConsoleSchematicFile::ConsoleSchematicFile()
    : mRefCount(1), mOrigin(0, 0, 0), mWidth(0), mHeight(0), mDepth(0), mIncludeAir(false), mIncludeMobs(false), mCopyArea(0), mUndoArea(0), mCopyMinX(0), mCopyMinZ(0), mCopyMaxX(0), mUndoWidth(0), mUndoHeight(0), mUndoDepth(0), mStoredCopyArea(0), mStoredUndoArea(0), mUndoReady(false), mPasteTicks(0), mPasteState(0), mHasCopy(false), mHasPaste(false), mMobs(false), mReserved(false)
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
    if (mirror == eSchematicMirror_None)
    {
        if (rotation == eSchematicRotation_90)
        {
            sx = (int)std::floor(x - box->mMinX);
            sz = (int)std::floor(box->mMaxZ - 1.0 - z);
        }
        else if (rotation == eSchematicRotation_270)
        {
            sx = (int)std::floor(box->mMaxX - 1.0 - x);
            sz = (int)std::floor(z - box->mMinZ);
        }
        else
        {
            sx = (int)std::floor(x - box->mMinX);
            sz = (int)std::floor(z - box->mMinZ);
        }
    }
    else if (mirror == eSchematicMirror_LeftRight)
    {
        if (rotation == eSchematicRotation_90)
        {
            sx = (int)std::floor(z - box->mMinZ);
            sz = (int)std::floor(x - box->mMinX);
        }
        else if (rotation == eSchematicRotation_270)
        {
            sx = (int)std::floor(box->mMaxZ - 1.0 - z);
            sz = (int)std::floor(box->mMaxX - 1.0 - x);
        }
        else
        {
            sx = (int)std::floor(z - box->mMinZ);
            sz = (int)std::floor(box->mMaxX - 1.0 - x);
        }
    }
    else if (mirror == eSchematicMirror_FrontBack)
    {
        if (rotation == eSchematicRotation_90)
        {
            sx = (int)std::floor(box->mMaxX - 1.0 - x);
            sz = (int)std::floor(z - box->mMinZ);
        }
        else if (rotation == eSchematicRotation_270)
        {
            sx = (int)std::floor(x - box->mMinX);
            sz = (int)std::floor(box->mMaxZ - 1.0 - z);
        }
        else
        {
            sx = (int)std::floor(box->mMaxX - 1.0 - x);
            sz = (int)std::floor(box->mMaxZ - 1.0 - z);
        }
    }
    else
    {
        if (rotation == eSchematicRotation_90)
        {
            sx = (int)std::floor(z - box->mMinZ);
            sz = (int)std::floor(x - box->mMinX);
        }
        else if (rotation == eSchematicRotation_270)
        {
            sx = (int)std::floor(box->mMaxZ - 1.0 - z);
            sz = (int)std::floor(x - box->mMinX);
        }
        else
        {
            sx = (int)std::floor(x - box->mMinX);
            sz = (int)std::floor(z - box->mMinZ);
        }
    }
}

void ConsoleSchematicFile::schematicCoordToChunkCoord(AABB* box, double x, double z, ESchematicRotation rotation, ESchematicMirror mirror, double& cx, double& cz)
{
    if (mirror == eSchematicMirror_None)
    {
        if (rotation == eSchematicRotation_90)
        {
            cx = std::floor(x + box->mMinX);
            cz = std::floor(box->mMaxZ - 1.0 - z);
        }
        else if (rotation == eSchematicRotation_270)
        {
            cx = std::floor(box->mMaxX - 1.0 - x);
            cz = std::floor(z + box->mMinZ);
        }
        else
        {
            cx = std::floor(x + box->mMinX);
            cz = std::floor(z + box->mMinZ);
        }
    }
    else if (mirror == eSchematicMirror_LeftRight)
    {
        if (rotation == eSchematicRotation_90)
        {
            cx = std::floor(z + box->mMinX);
            cz = std::floor(x + box->mMinZ);
        }
        else if (rotation == eSchematicRotation_270)
        {
            cx = std::floor(box->mMaxX - 1.0 - z);
            cz = std::floor(box->mMaxZ - 1.0 - x);
        }
        else
        {
            cx = std::floor(box->mMaxX - 1.0 - z);
            cz = std::floor(x + box->mMinZ);
        }
    }
    else if (mirror == eSchematicMirror_FrontBack)
    {
        if (rotation == eSchematicRotation_90)
        {
            cx = std::floor(box->mMaxX - 1.0 - x);
            cz = std::floor(z + box->mMinZ);
        }
        else if (rotation == eSchematicRotation_270)
        {
            cx = std::floor(x + box->mMinX);
            cz = std::floor(box->mMaxZ - 1.0 - z);
        }
        else
        {
            cx = std::floor(box->mMaxX - 1.0 - x);
            cz = std::floor(box->mMaxZ - 1.0 - z);
        }
    }
    else
    {
        if (rotation == eSchematicRotation_90)
        {
            cx = std::floor(z + box->mMinX);
            cz = std::floor(x + box->mMinZ);
        }
        else if (rotation == eSchematicRotation_270)
        {
            cx = std::floor(z + box->mMinX);
            cz = std::floor(box->mMaxZ - 1.0 - x);
        }
        else
        {
            cx = std::floor(x + box->mMinX);
            cz = std::floor(z + box->mMinZ);
        }
    }
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
    if (filename.size() != 0)
    {
        std::wstring extension(filename, filename.size() - 4);
        if (extension.compare(L".sch") != 0)
        {
            return filename + L".sch";
        }
    }
    return filename;
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
    if (input != 0)
    {
        delete[] mElements.data;
        mElements.data = 0;
        mElements.length = 0;
        delete[] mExtraElements.data;
        mExtraElements.data = 0;
        mExtraElements.length = 0;
        delete[] mData.data;
        mData.length = 0;
        mData.data = 0;
        delete[] mExtraData.data;
        mExtraData.length = 0;
        mExtraData.data = 0;

        int version = input->readInt();
        int compression = 2;
        if (version > 1)
            compression = input->readByte();
        mWidth = input->readInt();
        mHeight = input->readInt();
        mDepth = input->readInt();
        arrayWithLength<unsigned char> compressed(static_cast<unsigned int>(input->readInt()), true);
        input->read(compressed);
        Compression* compressor = Compression::getCompression();
        if (version < 3)
        {
            unsigned int legacySize = calcFullDataSizeLegacy(mWidth, mHeight, mDepth);
            arrayWithLength<unsigned char> data;
            unsigned int count;
            if (compression == 0)
            {
                data = compressed;
                count = calculateFullElementSize(mWidth, mHeight, mDepth);
            }
            else
            {
                data = arrayWithLength<unsigned char>(legacySize, true);
                mData = arrayWithLength<unsigned char>(legacySize, true);
                if (compression == 1)
                {
                    compressor->DecompressRLE(data.data, &data.length, compressed.data, compressed.length);
                }
                else if (compression == 3)
                {
                    compressor->DecompressLZXRLE(data.data, &data.length, compressed.data, compressed.length);
                }
                else
                {
                    compressor->m_decompressionType = static_cast<Compression::ECompressionTypes>(compression);
                    compressor->DecompressLZXRLE(data.data, &data.length, compressed.data, compressed.length);
                    compressor->m_decompressionType = Compression::eCompressionTypes_Zlib;
                }
                delete[] compressed.data;
                count = calculateFullElementSize(mWidth, mHeight, mDepth);
            }
            mElements = arrayWithLength<unsigned short>(count, true);
            mExtraElements = arrayWithLength<unsigned short>(count, true);
            unsigned int ids = calcBlockIdsSizeLegacy(mWidth, mHeight, mDepth);
            unsigned int nibble = ids;
            int column = 0;
            bool highNibble = true;
            for (unsigned int i = 0; i < ids; ++i)
            {
                highNibble = !highNibble;
                if (highNibble)
                {
                    mElements.data[i] = static_cast<unsigned short>(data.data[i] << 4 | data.data[nibble] >> 4);
                    ++nibble;
                }
                else
                {
                    mElements.data[i] = static_cast<unsigned short>(data.data[i] << 4 | (data.data[nibble] & 0xf));
                }
                if (++column >= mHeight)
                {
                    column = 0;
                    if ((mHeight & 1) != 0)
                    {
                        highNibble = true;
                        ++nibble;
                    }
                }
            }
            delete[] data.data;
        }
        else
        {
            unsigned int count = calculateFullElementSize(mWidth, mHeight, mDepth);
            if (compression == 0)
            {
                mElements = arrayWithLength<unsigned short>(reinterpret_cast<unsigned short*>(compressed.data), compressed.length);
                arrayWithLength<unsigned char> extra(static_cast<unsigned int>(input->readInt()), true);
                input->read(extra);
                mExtraElements = arrayWithLength<unsigned short>(reinterpret_cast<unsigned short*>(extra.data), extra.length);
            }
            else
            {
                mElements = arrayWithLength<unsigned short>(count, true);
                mExtraElements = arrayWithLength<unsigned short>(count, true);
                mData = arrayWithLength<unsigned char>(count, true);
                if (compression == 1)
                {
                    compressor->DecompressRLE(mElements.data, &mElements.length, compressed.data, compressed.length);
                }
                else if (compression == 3)
                {
                    compressor->DecompressLZXRLE(mElements.data, &mElements.length, compressed.data, compressed.length);
                }
                else
                {
                    compressor->m_decompressionType = static_cast<Compression::ECompressionTypes>(compression);
                    compressor->DecompressLZXRLE(mElements.data, &mElements.length, compressed.data, compressed.length);
                    compressor->m_decompressionType = Compression::eCompressionTypes_Zlib;
                }
                mElements.length >>= 1;
                delete[] compressed.data;
                arrayWithLength<unsigned char> extra(static_cast<unsigned int>(input->readInt()), true);
                input->read(extra);
                mExtraElements = arrayWithLength<unsigned short>(count, true);
                if (compression == 1)
                {
                    compressor->DecompressRLE(mExtraElements.data, &mExtraElements.length, extra.data, extra.length);
                }
                else if (compression == 3)
                {
                    compressor->DecompressLZXRLE(mExtraElements.data, &mExtraElements.length, extra.data, extra.length);
                }
                else
                {
                    compressor->m_decompressionType = static_cast<Compression::ECompressionTypes>(compression);
                    compressor->DecompressLZXRLE(mExtraElements.data, &mExtraElements.length, extra.data, extra.length);
                    compressor->m_decompressionType = Compression::eCompressionTypes_Zlib;
                }
                mExtraElements.length >>= 1;
                delete[] extra.data;
            }
        }
        mBlockEntities.clear();

        CompoundTag* tags = Minecraft::GetInstance()->getFixerUpper()->updateData(DataFixTypes::eDataFixTypes_Chunk, NbtIo::read(input));
        ListTag* tileEntities = tags->getList(L"TileEntities", Tag::TAG_COMPOUND);
        if (tileEntities != 0)
        {
            for (int i = 0; i < tileEntities->size(); ++i)
            {
                CompoundTag* tag = tileEntities->getCompound(i);
                boost::shared_ptr<BlockEntity> blockEntity = BlockEntity::loadStatic(0, tag);
                if (blockEntity)
                {
                    BlockPos* position = new BlockPos(tag->getInt(L"x"), tag->getInt(L"y"), tag->getInt(L"z"));
                    mBlockEntities.push_back(std::make_pair(position, tag->copy()));
                }
            }
        }
        mEntities.clear();
        ListTag* entities = tags->getList(L"Entities", Tag::TAG_COMPOUND);
        if (entities != 0)
        {
            for (int i = 0; i < entities->size(); ++i)
            {
                CompoundTag* tag = entities->getCompound(i);
                unsigned long long type = EntityIO::byString(tag->getString(L"id"));
                ListTag* position = tag->getList(L"Pos", Tag::TAG_DOUBLE);
                double x = position->getDouble(0);
                double y = position->getDouble(1);
                double z = position->getDouble(2);
                if (type == EntityType::PAINTING || type == EntityType::ITEM_FRAME)
                {
                    x = tag->getInt(L"TileX");
                    y = tag->getInt(L"TileY");
                    z = tag->getInt(L"TileZ");
                }
                mEntities.push_back(std::make_pair(Vec3::newPermanent(x, y, z), tag->copy()));
            }
        }
        mNamedPositions.clear();
        ListTag* namedPositions = tags->getList(L"NamedPositions", Tag::TAG_COMPOUND);
        if (namedPositions != 0)
        {
            for (int i = 0; i < namedPositions->size(); ++i)
            {
                CompoundTag* tag = namedPositions->getCompound(i);
                int x = tag->getInt(L"X");
                int y = tag->getInt(L"Y");
                int z = tag->getInt(L"Z");
                if (!tag->contains(L"X2"))
                {
                    std::string name = convWstringToString(tag->getString(L"Name"));
                    mNamedPositions.push_back(DevSaveDataManager::NamedPositionOrVolume(name, BlockPos(x, y, z)));
                }
                else
                {
                    int x2 = tag->getInt(L"X2");
                    int y2 = tag->getInt(L"Y2");
                    int z2 = tag->getInt(L"Z2");
                    std::string name = convWstringToString(tag->getString(L"Name"));
                    mNamedPositions.push_back(DevSaveDataManager::NamedPositionOrVolume(name, BlockPos(x, y, z), BlockPos(x2, y2, z2)));
                }
            }
        }
        delete tags;
    }
    mUndoReady = true;
}

void ConsoleSchematicFile::setElements(arrayWithLength<unsigned short>& out, arrayWithLength<unsigned short> in, ESchematicRotation rotation, ESchematicMirror mirror, int x0, int y0, int z0, int x1, int y1, int z1, int& count, bool includeAir, bool includeSpecial)
{
    bool hasLowY = false;
    bool hasHighY = false;
    int lowYEnd = y1;
    if (y0 < 0x80)
    {
        if (y1 > 0x80)
        {
            lowYEnd = 0x80;
        }
        hasLowY = true;
    }

    int highYStart = y0;
    int highYEnd = y1;
    if (y1 > 0x7f)
    {
        if (y0 < 0x80)
        {
            highYStart = 0x80;
        }
        highYStart -= 0x80;
        hasHighY = true;
        highYEnd = y1 - 0x80;
    }

    PIXBeginNamedEvent(0.0f, "Applying block data");
    for (int x = x0; x < x1; ++x)
    {
        for (int z = z0; z < z1; ++z)
        {
            if (mirror == eSchematicMirror_FrontBack)
            {
                count += (y1 - y0) - 1;
            }

            if (hasLowY)
            {
                int lowCount = lowYEnd - y0;
                if (mirror == eSchematicMirror_FrontBack)
                {
                    for (int y = y0; y < lowYEnd; ++y)
                    {
                        unsigned short element = in.data[count];
                        if (CanPasteBlock(element, includeAir, includeSpecial))
                        {
                            out.data[(unsigned int)((x << 11) | (z << 7) | y)] = rotateElementValue(element, rotation);
                        }
                        --count;
                    }
                }
                else if (includeAir && includeSpecial && rotation == eSchematicRotation_0)
                {
                    System::arraycopy(in, count, &out, (unsigned int)((x << 11) | (z << 7) | y0), lowCount);
                    count += lowCount;
                }
                else
                {
                    for (int y = y0; y < lowYEnd; ++y)
                    {
                        unsigned short element = in.data[count];
                        if (CanPasteBlock(element, includeAir, includeSpecial))
                        {
                            out.data[(unsigned int)((x << 11) | (z << 7) | y)] = rotateElementValue(element, rotation);
                        }
                        ++count;
                    }
                }
            }

            if (hasHighY)
            {
                int highCount = highYEnd - highYStart;
                if (mirror == eSchematicMirror_FrontBack)
                {
                    for (int y = highYStart; y < highYEnd; ++y)
                    {
                        unsigned short element = in.data[count];
                        if (CanPasteBlock(element, includeAir, includeSpecial))
                        {
                            out.data[(unsigned int)(((x << 11) | (z << 7) | y) + 0x8000)] = rotateElementValue(element, rotation);
                        }
                        --count;
                    }
                }
                else if (includeAir && includeSpecial && rotation == eSchematicRotation_0)
                {
                    System::arraycopy(in, count, &out, (unsigned int)(((x << 11) | (z << 7) | highYStart) + 0x8000), highCount);
                    count += highCount;
                }
                else
                {
                    for (int y = highYStart; y < highYEnd; ++y)
                    {
                        unsigned short element = in.data[count];
                        if (CanPasteBlock(element, includeAir, includeSpecial))
                        {
                            out.data[(unsigned int)(((x << 11) | (z << 7) | y) + 0x8000)] = rotateElementValue(element, rotation);
                        }
                        ++count;
                    }
                }
            }

            if (mirror == eSchematicMirror_FrontBack)
            {
                count += y1 - y0;
            }
        }
    }
    PIXEndNamedEvent();
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
    bool hasLowY = false;
    bool hasHighY = false;
    int lowYEnd = y1;
    if (y0 < 0x80)
    {
        if (y1 > 0x80)
        {
            lowYEnd = 0x80;
        }
        hasLowY = true;
    }

    int highYStart = y0;
    int highYEnd = y1;
    if (y1 > 0x7f)
    {
        if (y0 < 0x80)
        {
            highYStart = 0x80;
        }
        highYStart -= 0x80;
        hasHighY = true;
        highYEnd = y1 - 0x80;
    }

    PIXBeginNamedEvent(0.0f, "Applying Data data");
    for (int x = x0; x < x1; ++x)
    {
        for (int z = z0; z < z1; ++z)
        {
            if (mirror == eSchematicMirror_FrontBack)
            {
                index += (y1 - y0) - 1;
            }

            if (hasLowY)
            {
                if (mirror == eSchematicMirror_FrontBack)
                {
                    for (int y = y0; y < lowYEnd; y += 2)
                    {
                        unsigned int dest = (unsigned int)((x << 11) | (z << 7) | y) >> 1;
                        pasteIntoArray(out, in, elements, (index - 1) * 2, index - 1, dest, (y & 1) != 0, includeAir, includeSpecial);
                        index -= 2;
                        count += 2;
                    }
                }
                else
                {
                    for (int y = y0; y < lowYEnd; y += 2)
                    {
                        unsigned int dest = (unsigned int)((x << 11) | (z << 7) | y) >> 1;
                        pasteIntoArray(out, in, elements, index * 2, index, dest, (y & 1) != 0, includeAir, includeSpecial);
                        index += 2;
                        count += 2;
                    }
                }
            }

            if (hasHighY)
            {
                if (mirror == eSchematicMirror_FrontBack)
                {
                    for (int y = highYStart; y < highYEnd; y += 2)
                    {
                        unsigned int dest = (unsigned int)(((x << 11) | (z << 7) | y) + 0x8000) >> 1;
                        pasteIntoArray(out, in, elements, (index - 1) * 2, index - 1, dest, (y & 1) != 0, includeAir, includeSpecial);
                        index -= 2;
                        count += 2;
                    }
                }
                else
                {
                    for (int y = highYStart; y < highYEnd; y += 2)
                    {
                        unsigned int dest = (unsigned int)(((x << 11) | (z << 7) | y) + 0x8000) >> 1;
                        pasteIntoArray(out, in, elements, index * 2, index, dest, (y & 1) != 0, includeAir, includeSpecial);
                        index += 2;
                        count += 2;
                    }
                }
            }

            if (mirror == eSchematicMirror_FrontBack)
            {
                index += y1 - y0;
            }
        }
    }
    PIXEndNamedEvent();
}

void ConsoleSchematicFile::getElements(LevelChunk* chunk, arrayWithLength<unsigned short>* elements, arrayWithLength<unsigned short>* extra, int x0, int y0, int z0, int x1, int y1, int z1, int& count)
{
    bool hasLowY = false;
    bool hasHighY = false;
    int lowYEnd = y1;
    if (y0 < 0x80)
    {
        if (y1 > 0x80)
        {
            lowYEnd = 0x80;
        }
        hasLowY = true;
    }

    int highYStart = y0;
    int highYEnd = y1;
    if (y1 > 0x7f)
    {
        if (y0 < 0x80)
        {
            highYStart = 0x80;
        }
        highYStart -= 0x80;
        hasHighY = true;
        highYEnd = y1 - 0x80;
    }

    arrayWithLength<unsigned short> elementsBuffer(0x10000, true);
    arrayWithLength<unsigned short> extraBuffer(0x10000, true);
    chunk->getElementRange(arrayWithLength<unsigned short>(elementsBuffer.data, elementsBuffer.length), arrayWithLength<unsigned short>(extraBuffer.data, extraBuffer.length), 0, 0x40);

    count = 0;
    for (int x = x0; x < x1; ++x)
    {
        for (int z = z0; z < z1; ++z)
        {
            if (hasLowY)
            {
                int lowCount = lowYEnd - y0;
                unsigned int src = (unsigned int)((x << 11) | (z << 7) | y0);
                System::arraycopy(elementsBuffer, src, elements, count, lowCount);
                System::arraycopy(extraBuffer, src, extra, count, lowCount);
                count += lowCount;
            }
            if (hasHighY)
            {
                int highCount = highYEnd - highYStart;
                unsigned int src = (unsigned int)(((x << 11) | (z << 7) | highYStart) + 0x8000);
                System::arraycopy(elementsBuffer, src, elements, count, highCount);
                System::arraycopy(extraBuffer, src, extra, count, highCount);
                count += highCount;
            }
        }
    }

    delete[] elementsBuffer.data;
    delete[] extraBuffer.data;
}

void ConsoleSchematicFile::getData(arrayWithLength<unsigned char> in, arrayWithLength<unsigned char>* out, int& count, int x, int y, int z, int width, int height, int depth)
{
    bool hasLowY = false;
    bool hasHighY = false;
    int lowYEnd = height;
    if (y < 0x80)
    {
        if (height > 0x80)
        {
            lowYEnd = 0x80;
        }
        hasLowY = true;
    }

    unsigned int highY = y;
    if (height > 0x7f)
    {
        if (y < 0x80)
        {
            highY = 0x80;
        }
        height = height - 0x80;
        hasHighY = true;
        highY = highY - 0x80;
    }

    bool swapLowNibbles = hasLowY && (y & 1) != 0;
    bool swapHighNibbles = hasHighY && (highY & 1) != 0;
    bool oddLowSpan = hasLowY && ((lowYEnd - y) & 1) != 0;

    int xRemaining = width - x;
    if (x < width)
    {
        unsigned int xIndex = x << 11;
        unsigned char* source = in.data;
        do
        {
            if (z < depth)
            {
                unsigned int highYRemaining = height - highY;
                unsigned int lowYRemaining = lowYEnd - y;
                unsigned int zIndex = z << 7;
                int zRemaining = depth - z;
                do
                {
                    bool extractHigh = false;
                    if (hasLowY)
                    {
                        int dataIndex = (int)((xIndex | zIndex | (unsigned int)y) >> 1);
                        unsigned int yRemaining = lowYRemaining;
                        if (yRemaining > 1)
                        {
                            int outIndex = count;
                            unsigned char* packed = source + dataIndex - 1;
                            if (swapLowNibbles)
                            {
                                do
                                {
                                    out->data[outIndex] = (unsigned char)((packed[2] << 4) | (packed[1] >> 4));
                                    dataIndex = dataIndex + 1;
                                    yRemaining = yRemaining - 2;
                                    outIndex = count + 1;
                                    count = outIndex;
                                    packed = packed + 1;
                                }
                                while (yRemaining > 1);
                            }
                            else
                            {
                                do
                                {
                                    packed = packed + 1;
                                    out->data[outIndex] = *packed;
                                    yRemaining = yRemaining - 2;
                                    dataIndex = dataIndex + 1;
                                    outIndex = count + 1;
                                    count = outIndex;
                                }
                                while (yRemaining > 1);
                            }
                        }
                        if (yRemaining == 0)
                        {
                            extractHigh = hasHighY;
                        }
                        else
                        {
                            if (swapLowNibbles)
                            {
                                out->data[count] = source[dataIndex] >> 4;
                            }
                            else
                            {
                                out->data[count] = source[dataIndex] & 0xf;
                            }
                            if (hasHighY)
                            {
                                extractHigh = true;
                            }
                            else
                            {
                                ++count;
                            }
                        }
                    }
                    else
                    {
                        extractHigh = hasHighY;
                    }

                    if (extractHigh)
                    {
                        int dataIndex = (int)((xIndex | zIndex | highY) + 0x8000) >> 1;
                        if (oddLowSpan)
                        {
                            int outIndex = count;
                            unsigned char* outData = out->data;
                            if (swapHighNibbles)
                            {
                                outData[outIndex] = (unsigned char)((source[dataIndex] & 0xf0) | (outData[outIndex] & 0xf));
                                dataIndex = dataIndex + 1;
                                outIndex = count + 1;
                                count = outIndex;
                            }
                            else
                            {
                                outData[outIndex] = (unsigned char)((source[dataIndex] << 4) | (outData[outIndex] & 0xf));
                                outIndex = count + 1;
                                count = outIndex;
                            }
                            unsigned int yRemaining = highYRemaining - 1;
                            if (yRemaining > 1)
                            {
                                unsigned char* packed = source + dataIndex - 1;
                                if (swapHighNibbles)
                                {
                                    do
                                    {
                                        packed = packed + 1;
                                        yRemaining = yRemaining - 2;
                                        out->data[outIndex] = *packed;
                                        dataIndex = dataIndex + 1;
                                        outIndex = count + 1;
                                        count = outIndex;
                                    }
                                    while (yRemaining > 1);
                                }
                                else
                                {
                                    do
                                    {
                                        out->data[outIndex] = (unsigned char)((packed[1] >> 4) | (packed[2] << 4));
                                        yRemaining = yRemaining - 2;
                                        dataIndex = dataIndex + 1;
                                        outIndex = count + 1;
                                        count = outIndex;
                                        packed = packed + 1;
                                    }
                                    while (yRemaining > 1);
                                }
                            }
                            if (yRemaining != 0)
                            {
                                if (swapHighNibbles)
                                {
                                    out->data[outIndex] = source[dataIndex] & 0xf;
                                    ++count;
                                }
                                else
                                {
                                    out->data[outIndex] = source[dataIndex] >> 4;
                                    ++count;
                                }
                            }
                        }
                        else
                        {
                            unsigned int yRemaining = highYRemaining;
                            if (highYRemaining > 1)
                            {
                                int outIndex = count;
                                unsigned char* packed = source + dataIndex - 1;
                                if (swapHighNibbles)
                                {
                                    do
                                    {
                                        yRemaining = yRemaining - 2;
                                        out->data[outIndex] = (unsigned char)((packed[2] << 4) | (packed[1] >> 4));
                                        dataIndex = dataIndex + 1;
                                        outIndex = count + 1;
                                        count = outIndex;
                                        packed = packed + 1;
                                    }
                                    while (yRemaining > 1);
                                }
                                else
                                {
                                    do
                                    {
                                        packed = packed + 1;
                                        out->data[outIndex] = *packed;
                                        yRemaining = yRemaining - 2;
                                        dataIndex = dataIndex + 1;
                                        outIndex = count + 1;
                                        count = outIndex;
                                    }
                                    while (yRemaining > 1);
                                }
                            }
                            if (yRemaining != 0)
                            {
                                if (!swapHighNibbles)
                                {
                                    out->data[count] = source[dataIndex] & 0xf;
                                    ++count;
                                }
                                else
                                {
                                    out->data[count] = source[dataIndex] >> 4;
                                    ++count;
                                }
                            }
                        }
                    }

                    zRemaining = zRemaining - 1;
                    zIndex = zIndex + 0x80;
                }
                while (zRemaining != 0);
            }
            xRemaining = xRemaining - 1;
            xIndex = xIndex + 0x800;
        }
        while (xRemaining != 0);
    }
}

void ConsoleSchematicFile::fillArraysFromLevel(Level* level, int x, int y, int z, int width, int height, int depth, arrayWithLength<unsigned short>* elements, arrayWithLength<unsigned short>* extra, arrayWithLength<unsigned char>* data, arrayWithLength<unsigned char>* extraData)
{
    int yEnd = y + height;
    int x1 = x + width;
    int z1 = z + depth;

    arrayWithLength<unsigned char> skyLight(0x8000, true);
    arrayWithLength<unsigned char> extraSkyLight(0x8000, true);

    int elementCount = 0;
    int dataCount = 0;
    int extraDataCount = 0;

    for (int chunkX = x; chunkX < x1; chunkX = ((chunkX >> 4) << 4) + 16)
    {
        int lx0 = chunkX - ((chunkX >> 4) << 4);
        if (lx0 < 0) lx0 = 0;
        int lx1 = lx0 + 1;
        if (lx1 > 16) lx1 = 16;

        for (int chunkZ = z; chunkZ < z1; chunkZ = ((chunkZ >> 4) << 4) + 16)
        {
            int lz0 = chunkZ - ((chunkZ >> 4) << 4);
            int lz1 = z1 - ((chunkZ >> 4) << 4);
            if (lz0 < 0) lz0 = 0;
            if (lz1 > 16) lz1 = 16;

            LevelChunk* chunk = level->getChunk(chunkX >> 4, chunkZ >> 4);

            if (elements != 0)
            {
                getElements(chunk, elements, extra, lx0, y, lz0, lx1, yEnd, lz1, elementCount);
            }
            if (extraData != 0)
            {
                chunk->getBlockLightData(arrayWithLength<unsigned char>(extraSkyLight.data, extraSkyLight.length));
                getData(arrayWithLength<unsigned char>(extraSkyLight.data, extraSkyLight.length), extraData, extraDataCount, lx0, y, lz0, lx1, yEnd, lz1);
            }
            if (data != 0)
            {
                chunk->getSkyLightData(arrayWithLength<unsigned char>(skyLight.data, skyLight.length));
                getData(arrayWithLength<unsigned char>(skyLight.data, skyLight.length), data, dataCount, lx0, y, lz0, lx1, yEnd, lz1);
            }
        }
    }

    delete[] skyLight.data;
    delete[] extraSkyLight.data;
}

long long ConsoleSchematicFile::applyBlocksAndData(LevelChunk* chunk, PasteData* data)
{
    if (chunk == 0 || data == 0)
    {
        return 0;
    }

    int chunkMinX = chunk->x << 4;
    int chunkMinZ = chunk->z << 4;
    int x0 = (int)data->mArea->mMinX > chunkMinX ? (int)data->mArea->mMinX : chunkMinX;
    int z0 = (int)data->mArea->mMinZ > chunkMinZ ? (int)data->mArea->mMinZ : chunkMinZ;
    int x1 = (int)data->mArea->mMaxX < chunkMinX + 16 ? (int)data->mArea->mMaxX : chunkMinX + 16;
    int z1 = (int)data->mArea->mMaxZ < chunkMinZ + 16 ? (int)data->mArea->mMaxZ : chunkMinZ + 16;
    long long blocksApplied = writeSchematicIntoArrays(chunkMinX, 0, chunkMinZ, x0, (int)data->mArea->mMinY, z0, x1, (int)data->mArea->mMaxY, z1, data, data->mElements, data->mExtraElements, data->mData, data->mExtraData);
    chunk->setAllElements(data->mElements, data->mExtraElements);
    return blocksApplied;
}

long long ConsoleSchematicFile::applyBlocksAndData(LevelChunk* chunk, AABB* box, ESchematicRotation rotation, ESchematicMirror mirror)
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
    return applyBlocksAndData(chunk, &data);
}
void ConsoleSchematicFile::applyBlockEntities(LevelChunk* chunk, AABB* chunkArea, PasteData* paste, Level* level)
{
    for (std::vector<std::pair<BlockPos*, CompoundTag*> >::iterator it = paste->mBlockEntities.begin(); it != paste->mBlockEntities.end(); ++it)
    {
        double x = it->first->x;
        double y = it->first->y + static_cast<int>(std::floor(paste->mArea->mMinY));
        double z = it->first->z;
        schematicCoordToChunkCoord(paste->mArea, x, z, paste->mRotation, paste->mMirror, x, z);
        if (chunkArea->containsIncludingLowerBound(Vec3::newTemp(x + 0.01, y + 0.01, z + 0.01)))
        {
            boost::shared_ptr<BlockEntity> blockEntity = BlockEntity::loadStatic(level, it->second);
            if (blockEntity)
            {
                boost::shared_ptr<SignBlockEntity> sign = boost::dynamic_pointer_cast<SignBlockEntity>(blockEntity);
                if (sign)
                {
                    sign->SetVerified(true);
                    sign->SetCensored(false);
                    sign->setCreatedByHostPlayer(true);
                    sign->setCreatedByRestrictedPlayer(false);
                }
                blockEntity->setPosition(BlockPos(static_cast<int>(x), static_cast<int>(y), static_cast<int>(z)));
                chunk->addBlockEntity(blockEntity);
                blockEntity->setChanged();
            }
        }
    }

    for (std::vector<std::pair<Vec3*, CompoundTag*> >::iterator it = paste->mEntities.begin(); it != paste->mEntities.end(); ++it)
    {
        double x = it->first->x;
        double y = it->first->y + paste->mArea->mMinY;
        double z = it->first->z;
        schematicCoordToChunkCoord(paste->mArea, x, z, paste->mRotation, paste->mMirror, x, z);
        if (!chunkArea->containsIncludingLowerBound(Vec3::newTemp(x + 0.01, y + 0.01, z + 0.01)))
            continue;

        CompoundTag* tag = it->second;
        boost::shared_ptr<Entity> entity = EntityIO::newEntity(tag, chunk->level);
        if (entity->GetType() == EntityType::PAINTING)
        {
            boost::shared_ptr<Painting> painting = boost::dynamic_pointer_cast<Painting>(entity);
            painting->setPos(x, y, z);
            painting->setDirection(Direction::from2DDataValue(tag->getByte(L"Facing")));
            offsetPainting(level, painting, paste->mRotation, paste->mMirror);
        }
        else
        {
            entity->absMoveTo(x, y, z, entity->mYRot, entity->mXRot);
        }
        entity->setLevel(chunk->level);
        entity->setDespawnProtected();
        chunk->level->addEntity(entity);
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

long long ConsoleSchematicFile::applyLighting(LevelChunk* chunk, AABB*, AABB* area, ESchematicRotation)
{
    if (chunk == 0 || area == 0)
    {
        return 0;
    }

    int chunkMinX = chunk->x << 4;
    int chunkMinZ = chunk->z << 4;
    int minX = (int)std::floor(area->mMinX);
    int maxX = (int)std::floor(area->mMaxX);
    int minY = (int)std::floor(area->mMinY);
    int maxY = (int)std::floor(area->mMaxY);
    int minZ = (int)std::floor(area->mMinZ);
    int maxZ = (int)std::floor(area->mMaxZ);

    if (minX < chunkMinX) minX = chunkMinX;
    if (maxX > chunkMinX + 16) maxX = chunkMinX + 16;
    if (minZ < chunkMinZ) minZ = chunkMinZ;
    if (maxZ > chunkMinZ + 16) maxZ = chunkMinZ + 16;

    int volume = (maxX - minX) * (maxY - minY) * (maxZ - minZ);

    int startY = minY;
    if (minX < maxX && minZ < maxZ)
    {
        for (int x = minX; x < maxX; ++x)
        {
            for (int z = minZ; z < maxZ; ++z)
            {
                int height = chunk->getHeightmap(x - chunkMinX, z - chunkMinZ);
                if (height < startY)
                {
                    startY = chunk->getHeightmap(x - chunkMinX, z - chunkMinZ);
                }
            }
        }
    }

    chunk->recalcHeightmap();

    if (minX < maxX && minZ < maxZ)
    {
        for (int x = minX; x < maxX; ++x)
        {
            for (int y = startY; y < maxY; ++y)
            {
                for (int z = minZ; z < maxZ; ++z)
                {
                    BlockPos pos(x, y, z);
                    if (y <= chunk->getHeightmap(x - chunkMinX, z - chunkMinZ))
                    {
                        chunk->level->checkLight(LightLayer::SKY, pos, true, false);
                    }
                    chunk->getBlock(BlockPos(x - chunkMinX, y, z - chunkMinZ));
                }
            }
        }
    }

    return volume;
}

void ConsoleSchematicFile::offsetPainting(Level*, boost::shared_ptr<Painting> painting, ESchematicRotation rotation, ESchematicMirror mirror)
{
    const Direction* direction = HandleRotation(painting->getDirection(), mirror, rotation);
    painting->setDirection(direction);

    if ((mirror == eSchematicMirror_LeftRight || mirror == eSchematicMirror_Both) && painting->getWidth() > 16)
    {
        switch (direction->get3DDataValue())
        {
        case Direction::eDirection_North:
            painting->mX += 1.0;
            break;
        case Direction::eDirection_South:
            painting->mX -= 1.0;
            break;
        case Direction::eDirection_West:
            painting->mZ -= 1.0;
            break;
        case Direction::eDirection_East:
            painting->mZ += 1.0;
            break;
        }
    }
}
void ConsoleSchematicFile::_applySchematicData(Level* level, AABB* area, ESchematicRotation rotation, arrayWithLength<unsigned short> elements, arrayWithLength<unsigned short> extraElements, arrayWithLength<unsigned char> data, arrayWithLength<unsigned char> extraData, std::vector<std::pair<BlockPos*, CompoundTag*> >& blockEntities, std::vector<std::pair<Vec3*, CompoundTag*> >& entities, std::vector<DevSaveDataManager::NamedPositionOrVolume>& namedPositions, int width, int height, int depth, bool includeAir, bool includeSpecial, bool rotate)
{
    mPasteData.mLevel = level;
    mPasteData.mArea->set(area);
    mPasteData.mRotation = rotate ? rotation : eSchematicRotation_0;
    mPasteData.mMirror = rotate ? (ESchematicMirror)mPasteState : eSchematicMirror_None;
    mPasteData.mMinChunkX = (int)std::floor(area->mMinX) >> 4;
    mPasteData.mMaxChunkX = (int)std::floor(area->mMaxX) >> 4;
    mPasteData.mCurrentChunkX = mPasteData.mMinChunkX;
    mPasteData.mMinChunkZ = (int)std::floor(area->mMinZ) >> 4;
    mPasteData.mMaxChunkZ = (int)std::floor(area->mMaxZ) >> 4;
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
    if (!mIncludeAir)
    {
        return false;
    }

    Minecraft::GetInstance()->mGameRenderer->DisableUpdateThread(0);

    int minX = (int)std::floor(mCopyArea->mMinX);
    int maxX = (int)std::floor(mCopyArea->mMaxX);
    int minY = (int)std::floor(mCopyArea->mMinY);
    int maxY = (int)std::floor(mCopyArea->mMaxY);
    int minZ = (int)std::floor(mCopyArea->mMinZ);
    int maxZ = (int)std::floor(mCopyArea->mMaxZ);
    for (int x = minX; x < maxX; ++x)
    {
        for (int y = minY; y < maxY; ++y)
        {
            for (int z = minZ; z < maxZ; ++z)
            {
                BlockPos pos(x, y, z);
                level->getChunkAt(pos)->removeBlockEntity(pos);
            }
        }
    }

    AABB* bounds = AABB::newTemp(mCopyArea->mMinX, mCopyArea->mMinY, mCopyArea->mMinZ, mCopyArea->mMaxX, mCopyArea->mMaxY, mCopyArea->mMaxZ);
    std::vector<boost::shared_ptr<Entity> > entities;
    level->getEntities(boost::shared_ptr<Entity>(), bounds, &entities);
    for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities.begin(); it != entities.end(); ++it)
    {
        level->removeEntity(*it);
    }

    _applySchematicData(level, mCopyArea, eSchematicRotation_0, mUndoElements, mUndoExtraElements, mData, mExtraData, mUndoBlockEntities, mUndoEntities, mUndoNamedPositions, mUndoWidth, mUndoHeight, mUndoDepth, true, true, false);

    Minecraft::GetInstance()->mGameRenderer->EnableUpdateThread(0);
    mIncludeAir = false;
    return true;
}
long long ConsoleSchematicFile::writeSchematicIntoArrays(int chunkX, int chunkY, int chunkZ, int x0, int y0, int z0, int x1, int y1, int z1, PasteData* paste, arrayWithLength<unsigned short> elements, arrayWithLength<unsigned short> extraElements, arrayWithLength<unsigned char> data, arrayWithLength<unsigned char> extraData)
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
    return volume;
}
void ConsoleSchematicFile::tick()
{
    if (!mHasCopy)
    {
        return;
    }

    if (mPasteData.mCurrentChunkX > mPasteData.mMaxChunkX || mPasteData.mCurrentChunkZ > mPasteData.mMaxChunkZ)
    {
        for (int x = (int)std::floor(mPasteData.mArea->mMinX); x < (int)std::floor(mPasteData.mArea->mMaxX); ++x)
        {
            for (int y = (int)std::floor(mPasteData.mArea->mMinY); y < (int)std::floor(mPasteData.mArea->mMaxY); ++y)
            {
                if (y >= SCHEMATIC_WORLD_HEIGHT)
                    continue;

                for (int z = (int)std::floor(mPasteData.mArea->mMinZ); z < (int)std::floor(mPasteData.mArea->mMaxZ); ++z)
                {
                    BlockPos pos(x, y, z);
                    if (mPasteData.mMirror == eSchematicMirror_None && mPasteData.mRotation == eSchematicRotation_0)
                        continue;

                    std::vector<const Property*> properties(mPasteData.mLevel->getBlockState(pos)->getProperties());
                    for (std::vector<const Property*>::iterator it = properties.begin(); it != properties.end(); ++it)
                    {
                        const DirectionProperty* property = dynamic_cast<const DirectionProperty*>(*it);
                        if (property == NULL)
                            continue;

                        const Direction* direction =
                            HandleRotation(mPasteData.mLevel->getBlockState(pos)->getValue(property), mPasteData.mMirror, eSchematicRotation_0);
                        TypedBoxed<const Direction*> boxed(direction);
                        mPasteData.mLevel->setBlockAndUpdate(pos, mPasteData.mLevel->getBlockState(pos)->setBoxedValue(property, &boxed));
                    }
                }
            }
        }
        mHasCopy = false;
        return;
    }

    LevelChunk* chunk = mPasteData.mLevel->getChunk(mPasteData.mCurrentChunkX, mPasteData.mCurrentChunkZ);
    if (!chunk->isEmpty())
    {
        double minX = mPasteData.mCurrentChunkX * 16.0;
        double minZ = mPasteData.mCurrentChunkZ * 16.0;
        AABB* chunkArea = AABB::newTemp(minX, 0.0, minZ, minX + 16.0, 255.0, minZ + 16.0);
        applyBlocksAndData(chunk, &mPasteData);
        applyLighting(chunk, chunkArea, mPasteData.mArea, mPasteData.mRotation);
        applyBlockEntities(chunk, chunkArea, &mPasteData, mPasteData.mLevel);

        int minY = (int)std::floor(mPasteData.mArea->mMinY);
        int maxY = (int)std::floor(mPasteData.mArea->mMaxY);
        for (int x = (int)minX; x < (int)minX + 16; ++x)
        {
            for (int y = minY; y <= maxY && y < SCHEMATIC_WORLD_HEIGHT; ++y)
            {
                for (int z = (int)minZ; z < (int)minZ + 16; ++z)
                {
                    BlockPos pos(x, y, z);
                    static_cast<ServerLevel*>(mPasteData.mLevel)->getChunkMap()->blockChanged(pos);
                }
            }
        }
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
    AABB* bounds = AABB::newTemp((double)x0, (double)y0, (double)z0, (double)x1, (double)y1, (double)z1);
    std::vector<boost::shared_ptr<Entity> > entities;
    level->getEntities(boost::shared_ptr<Entity>(), bounds, &entities);
    for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities.begin(); it != entities.end(); ++it)
    {
        boost::shared_ptr<Entity> entity = *it;
        if (entity == NULL)
            continue;

        unsigned long long type = entity->GetType();
        if (!mMobs && (type & EntityType::MOB) == EntityType::MOB)
            continue;

        CompoundTag* tag = new CompoundTag();
        if (entity->save(tag))
        {
            if (tag->contains(L"UUID"))
            {
                tag->remove(L"UUID");
            }

            ListTag* position = tag->getList(L"Pos", Tag::TAG_DOUBLE);
            if (position != NULL && position->size() >= 3)
            {
                position->set(0, new DoubleTag(entity->mX - x0));
                position->set(1, new DoubleTag(entity->mY - y0));
                position->set(2, new DoubleTag(entity->mZ - z0));
            }

            if (type == EntityType::PAINTING || type == EntityType::ITEM_FRAME)
            {
                tag->putInt(L"TileX", (int)(entity->mX - x0));
                tag->putInt(L"TileY", (int)(entity->mY - y0));
                tag->putInt(L"TileZ", (int)(entity->mZ - z0));
            }
            result.push_back(std::make_pair(Vec3::newPermanent(entity->mX - x0, entity->mY - y0, entity->mZ - z0), tag));
        }
        else
        {
            delete tag;
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
