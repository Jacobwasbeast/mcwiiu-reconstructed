#ifndef GAME_CORE_VERTEXFORMATELEMENT_HPP
#define GAME_CORE_VERTEXFORMATELEMENT_HPP

#include <string>

class VertexFormatElement
{
public:
    class Type
    {
    public:
        enum EType
        {
            eType_Float,
            eType_UnsignedByte,
            eType_Byte,
            eType_UnsignedShort,
            eType_Short,
            eType_UnsignedInt,
            eType_Int
        };

        static const Type* FLOAT;
        static const Type* UBYTE;
        static const Type* BYTE;
        static const Type* USHORT;
        static const Type* SHORT;
        static const Type* UINT;
        static const Type* INT;

        Type(int size, std::wstring name, int glType, EType id);

        int getSize() const;
        std::wstring getName() const;
        int getGlType() const;
        EType getId() const;

    private:
        int size;
        std::wstring name;
        int glType;
        EType id;
    };

    class Usage
    {
    public:
        enum EUsage
        {
            eUsage_Position,
            eUsage_Normal,
            eUsage_Color,
            eUsage_Uv,
            eUsage_BoneMatrix,
            eUsage_BlendWeight,
            eUsage_Padding
        };

        static const Usage* POSITION;
        static const Usage* NORMAL;
        static const Usage* COLOR;
        static const Usage* UV;
        static const Usage* BONE_MATRIX;
        static const Usage* BLEND_WEIGHT;
        static const Usage* PADDING;

        Usage(std::wstring name, EUsage id);

        std::wstring getName() const;
        EUsage getId() const;

    private:
        std::wstring name;
        EUsage id;
    };

    VertexFormatElement(const VertexFormatElement& other);
    VertexFormatElement(int index, const Type* type, const Usage* usage, int count);

    const Type* getType() const;
    const Usage* getUsage() const;
    int getIndex() const;
    int getCount() const;
    int getByteSize() const;
    bool isPosition() const;
    bool equals(VertexFormatElement* other) const;
    std::wstring toString() const;

private:
    bool supportsUsage(int index, const Usage* usage) const;

    const Type* type;
    const Usage* usage;
    int index;
    int count;
};

#endif
