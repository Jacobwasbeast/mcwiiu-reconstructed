#include "game/data/Tag.hpp"

#include "game/data/ByteArrayTag.hpp"
#include "game/data/ByteTag.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/DoubleTag.hpp"
#include "game/data/EndTag.hpp"
#include "game/data/FloatTag.hpp"
#include "game/data/IntArrayTag.hpp"
#include "game/data/IntTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/data/LongTag.hpp"
#include "game/data/ShortTag.hpp"
#include "game/data/StringTag.hpp"

#include <new>
#include <ostream>
#include <string>

static const int NUM_TAG_TYPES = 12;

static const wchar_t* const TAG_TYPE_NAMES[NUM_TAG_TYPES + 1] =
{
    L"TAG_End",
    L"TAG_Byte",
    L"TAG_Short",
    L"TAG_Int",
    L"TAG_Long",
    L"TAG_Float",
    L"TAG_Double",
    L"TAG_Byte_Array",
    L"TAG_String",
    L"TAG_List",
    L"TAG_Compound",
    L"TAG_Int_Array",
    L"UNKNOWN"
};

Tag::Tag()
{
    Tag* obj = this;
    if (obj != NULL || (obj = static_cast<Tag*>(operator new(4)), obj != NULL))
    {
    }
}

Tag::~Tag()
{
}

const wchar_t* Tag::getTagTypeName(unsigned char id)
{
    if (id < NUM_TAG_TYPES)
    {
        return TAG_TYPE_NAMES[id];
    }

    if (id == 99)
    {
        return L"Any Numeric Tag";
    }

    return TAG_TYPE_NAMES[NUM_TAG_TYPES];
}

void Tag::print(const std::wstring& name, char* indent, std::wostream& out)
{
    out << indent;
    out << getTagTypeName(getId());

    if (name.length() > 0)
    {
        out << L"(\"" << name << L"\")";
    }

    out << L": ";
    out << toString() << std::endl;
}

Tag* Tag::newTag(unsigned char id)
{
    switch (id)
    {
    case 0:
        return new EndTag();
    case 1:
        return new ByteTag();
    case 2:
        return new ShortTag();
    case 3:
        return new IntTag();
    case 4:
        return new LongTag();
    case 5:
        return new FloatTag();
    case 6:
        return new DoubleTag();
    case 7:
        return new ByteArrayTag();
    case 8:
    {
        void* mem = operator new(0x24);
        if (mem == NULL)
            return NULL;
        return new (mem) StringTag();
    }
    case 9:
    {
        void* mem = operator new(0x18);
        if (mem == NULL)
            return NULL;
        return new (mem) ListTag(0);
    }
    case 10:
        return new CompoundTag();
    case 11:
        return new IntArrayTag();
    default:
        return NULL;
    }
}

bool Tag::equals(Tag* other)
{
    if (other == NULL)
    {
        return false;
    }

    return getId() == other->getId();
}

void Tag::stripEmptyChildren()
{
}

bool Tag::isEmpty() const
{
    return false;
}

std::wstring Tag::getAsString()
{
    return toString();
}
