#include "game/data/ListTag.hpp"

#include "game/core/DataInput.hpp"
#include "game/core/DataOutput.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/EndTag.hpp"
#include "game/data/IntArrayTag.hpp"
#include "game/data/NumericTag.hpp"
#include "game/data/Tag.hpp"

#include <cstring>
#include <cwchar>
#include <ostream>
#include <string>
#include <vector>

template <class T>
void deleteElements(const std::vector<T*, std::allocator<T*> >& elements);

ListTag::ListTag(int type)
    : mList(), mType(type)
{
}

ListTag::~ListTag()
{
    deleteElements(mList);
}

void ListTag::write(DataOutput* output)
{
    if (size() == 0)
    {
        mType = 0;
    }
    else
    {
        mType = mList[0]->getId();
    }

    output->writeByte(mType);
    output->writeInt(size());

    for (std::vector<Tag*>::iterator it = mList.begin(); it != mList.end(); ++it)
    {
        (*it)->write(output);
    }
}

void ListTag::load(DataInput* input, int depth)
{
    if (depth > 512)
    {
        return;
    }

    mType = input->readByte();

    int count = input->readInt();

    deleteElements(mList);
    mList.clear();

    for (int i = 0; i < count; ++i)
    {
        Tag* tag = Tag::newTag(mType);
        tag->load(input, depth);
        mList.push_back(tag);
    }
}

unsigned char ListTag::getId()
{
    return 9;
}

std::wstring ListTag::toString()
{
    static wchar_t buffer[64];

    swprintf(buffer, 64, L"%zd entries of type %ls", mList.size(), Tag::getTagTypeName(mType));

    return buffer;
}

void ListTag::print(const std::wstring& name, char* indent, std::wostream& out)
{
    Tag::print(name, indent, out);

    out << indent << L"{" << std::endl;

    char* childIndent = new char[strlen(indent) + 4];
    strcpy(childIndent, indent);
    strcat(childIndent, "   ");

    for (std::vector<Tag*>::iterator it = mList.begin(); it != mList.end(); ++it)
    {
        (*it)->print(name, childIndent, out);
    }

    delete[] childIndent;

    out << indent << "}" << std::endl;
}

void ListTag::add(Tag* tag)
{
    if (tag->getId() == 0)
    {
        return;
    }

    if (mType == 0)
    {
        mType = tag->getId();
    }
    else if (mType != tag->getId())
    {
        if (size() != 0)
        {
            return;
        }

        mType = tag->getId();
    }

    mList.push_back(tag);
}

void ListTag::set(int index, Tag* tag)
{
    if (tag->getId() == 0)
    {
        return;
    }

    if (index < 0 || index >= size())
    {
        return;
    }

    if (mType == 0)
    {
        mType = tag->getId();
    }
    else if (mType != tag->getId())
    {
        if (size() != 0 || index != 0)
        {
            return;
        }

        mType = tag->getId();
    }

    if (mList[index] != tag)
    {
        delete mList[index];
        mList[index] = tag;
    }
}

Tag* ListTag::get(int index) const
{
    if (index >= 0 && index < size())
    {
        return mList.at(index);
    }

    return new EndTag();
}

Tag* ListTag::remove(int index)
{
    Tag* tag = get(index);

    mList.erase(mList.begin() + index);

    return tag;
}

void ListTag::stripEmptyChildren()
{
    std::vector<Tag*>::iterator it = mList.begin();

    while (it != mList.end())
    {
        Tag* tag = *it;

        if (tag->isEmpty())
        {
            delete tag;
            it = mList.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

bool ListTag::isEmpty() const
{
    return size() == 0;
}

CompoundTag* ListTag::getCompound(int index) const
{
    if (index >= 0 && index < size())
    {
        Tag* tag = mList.at(index);

        if (tag->getId() == 10)
        {
            return (CompoundTag*)tag;
        }
    }

    return new CompoundTag();
}

int ListTag::getInt(int index) const
{
    if (index >= 0 && index < size())
    {
        Tag* tag = mList.at(index);

        if (tag->getId() == 3)
        {
            return ((NumericTag*)tag)->getAsInt();
        }
    }

    return 0;
}

arrayWithLength<int> ListTag::getIntArray(int index) const
{
    if (index >= 0 && index < size())
    {
        Tag* tag = mList.at(index);

        if (tag->getId() == 11)
        {
            return ((IntArrayTag*)tag)->getAsIntArray();
        }
    }

    return arrayWithLength<int>();
}

double ListTag::getDouble(int index) const
{
    if (index >= 0 && index < size())
    {
        Tag* tag = mList.at(index);

        if (tag->getId() == 6)
        {
            return ((NumericTag*)tag)->getAsDouble();
        }
    }

    return 0.0;
}

float ListTag::getFloat(int index) const
{
    if (index >= 0 && index < size())
    {
        Tag* tag = mList.at(index);

        if (tag->getId() == 5)
        {
            return ((NumericTag*)tag)->getAsFloat();
        }
    }

    return 0.0f;
}

std::wstring ListTag::getString(int index) const
{
    if (index >= 0 && index < size())
    {
        Tag* tag = mList.at(index);

        if (tag->getId() != 8)
        {
            return tag->toString();
        }

        return tag->getAsString();
    }

    return L"";
}

ListTag* ListTag::copy()
{
    ListTag* list = new ListTag(0);

    list->mType = mType;

    for (std::vector<Tag*>::iterator it = mList.begin(); it != mList.end(); ++it)
    {
        Tag* tag = (*it)->copy();
        list->mList.push_back(tag);
    }

    return list;
}

bool ListTag::equals(Tag* other)
{
    if (!Tag::equals(other))
    {
        return false;
    }

    ListTag* list = (ListTag*)other;

    if (mType != list->mType)
    {
        return false;
    }

    if (size() != list->size())
    {
        return false;
    }

    for (std::vector<Tag*>::const_iterator it = mList.begin(), other_it = list->mList.begin(); it != mList.end(); ++it, ++other_it)
    {
        if (!(*it)->equals(*other_it))
        {
            return false;
        }
    }

    return true;
}

unsigned char ListTag::getType()
{
    return mType;
}
