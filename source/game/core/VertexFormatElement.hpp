#ifndef GAME_CORE_VERTEX_FORMAT_ELEMENT_HPP
#define GAME_CORE_VERTEX_FORMAT_ELEMENT_HPP

#include <string>

class VertexFormatElement
{
public:
    class Type
    {
    public:
        enum EType : int;
        Type(int size, const std::wstring& name, int glType, EType type);
        int getSize() const;
        std::wstring getName() const;
        int getGlType() const;
        int getId() const;
    private:
        int size;
        std::wstring name;
        int glType;
        EType type;
    };

    class Usage
    {
    public:
        enum EUsage : int;
        Usage(const std::wstring& name, EUsage usage);
        int getId() const;
        std::wstring getName() const;
    private:
        std::wstring name;
        EUsage usage;
    };

    VertexFormatElement(const VertexFormatElement& source);
    VertexFormatElement(int elementIndex, const Type* elementType,
                        const Usage* elementUsage, int elementCount);
    bool isPosition() const;
    const Usage* getUsage() const;
    int getIndex() const;
    int getByteSize() const;
    bool supportsUsage(int elementIndex, const Usage* elementUsage) const;
    std::wstring toString() const;
    const Type* getType() const;
    int getCount() const;
    bool equals(const VertexFormatElement* other) const;


private:
    const Type* type;
    const Usage* usage;
    int index;
    int count;
};

#endif
