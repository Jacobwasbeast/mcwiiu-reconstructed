#ifndef GAME_DATA_TAG_HPP
#define GAME_DATA_TAG_HPP

#include <iosfwd>
#include <string>

class DataInput;
class DataOutput;

class Tag
{
public:
    Tag();

    virtual void write(DataOutput* output) = 0;
    virtual void load(DataInput* input, int depth) = 0;
    virtual std::wstring toString() = 0;
    virtual unsigned char getId() = 0;
    virtual void print(const std::wstring& name, char* indent, std::wostream& out);
    virtual ~Tag();
    virtual bool equals(Tag* other);
    virtual Tag* copy() = 0;
    virtual void stripEmptyChildren();
    virtual bool isEmpty() const;
    virtual std::wstring getAsString();

    static const wchar_t* getTagTypeName(unsigned char id);
    static Tag* newTag(unsigned char id);
};

#endif
